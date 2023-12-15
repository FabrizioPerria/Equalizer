#pragma once

#include "data/FilterParameters.h"
#include "utils/Decibel.h"
#include "utils/Fifo.h"
#include "utils/FilterCoefficientGenerator.h"
#include "utils/ReleasePool.h"
#include <JuceHeader.h>

using Filter = juce::dsp::IIR::Filter<float>;
using Coefficients = juce::dsp::IIR::Coefficients<float>;
using CoefficientsPtr = Coefficients::Ptr;
using CutCoefficients = juce::ReferenceCountedArray<Coefficients>;
using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

template <typename FilterType>
struct IsCutFilter : std::false_type
{
};

template <>
struct IsCutFilter<CutFilter> : std::true_type
{
};

template <typename FilterType>
struct IsParametricFilter : std::false_type
{
};

template <>
struct IsParametricFilter<Filter> : std::true_type
{
};

template <typename ParamType>
struct IsCutParameter : std::false_type
{
};

template <>
struct IsCutParameter<HighCutLowCutParameters> : std::true_type
{
};

template <typename FilterType, typename FifoDataType, typename ParamType, typename FunctionType>
struct FilterLink
{
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        filter.prepare (spec);
    }

    void reset()
    {
        filter.reset();
    }

    template <typename ContextType>
    void process (const ContextType& context)
    {
        if (currentParams.bypassed)
        {
            return;
        }

        filter.process (context);
    }

    void updateSmootherTargets()
    {
        if (! juce::approximatelyEqual (currentParams.frequency, freqSmoother.getTargetValue()))
        {
            freqSmoother.setTargetValue (currentParams.frequency);
        }

        if (! juce::approximatelyEqual (currentParams.quality, qualitySmoother.getTargetValue()))
        {
            qualitySmoother.setTargetValue (currentParams.quality);
        }

        if constexpr (! IsCutParameter<ParamType>::value)
        {
            if (! juce::approximatelyEqual (currentParams.gain, gainSmoother.getTargetValue()))
            {
                gainSmoother.setTargetValue (currentParams.gain);
            }
        }
    }

    void resetSmoothers (float rampTime)
    {
        freqSmoother.reset (sampleRate, rampTime);
        freqSmoother.setCurrentAndTargetValue (currentParams.frequency);

        qualitySmoother.reset (sampleRate, rampTime);
        qualitySmoother.setCurrentAndTargetValue (currentParams.quality);

        if constexpr (! IsCutParameter<ParamType>::value)
        {
            gainSmoother.reset (sampleRate, rampTime);
            gainSmoother.setCurrentAndTargetValue (currentParams.gain);
        }
    }

    bool isSmoothing() const
    {
        auto freqSmoothing = freqSmoother.isSmoothing();
        auto qualitySmoothing = qualitySmoother.isSmoothing();
        auto gainSmoothing = false;
        if constexpr (! IsCutParameter<ParamType>::value)
        {
            gainSmoothing = gainSmoother.isSmoothing();
        }
        return freqSmoothing || qualitySmoothing || gainSmoothing;
    }

    void checkIfStillSmoothing()
    {
        if (isSmoothing())
        {
            shouldComputeNewCoefficients = true;
        }
    }

    void advanceSmoothers (int numSamples)
    {
        freqSmoother.skip (numSamples);
        qualitySmoother.skip (numSamples);
        if constexpr (! IsCutParameter<ParamType>::value)
        {
            gainSmoother.skip (numSamples);
        }
    }

    void updateParams (const ParamType& params)
    {
        if (params != currentParams)
        {
            shouldComputeNewCoefficients = true;
            currentParams = params;
        }
    }

    void updateCoefficients (const FifoDataType& coefficents)
    {
        if constexpr (IsCutFilter<FilterType>::value)
        {
            configureCutFilterChain (coefficents);
        }
        else
        {
            updateFilterState (filter.coefficients, coefficents);
        }
    }

    void loadCoefficients (bool fromFifo)
    {
        if (fromFifo)
        {
            if (coefficientsFifo.getNumAvailableForReading() > 0)
            {
                FifoDataType coefficients;
                discardOldCoefficientsIfAny();
                auto success = coefficientsFifo.pull (coefficients);
                jassert (success);
                updateCoefficients (coefficients);
            }
        }
        else
        {
            FifoDataType coefficients = FunctionType::make (currentParams);
            updateCoefficients (coefficients);
        }
    }

    void generateNewCoefficientsIfNeeded()
    {
        if (shouldComputeNewCoefficients.compareAndSetBool (false, true))
        {
            ParamType params;

            params = currentParams;
            params.frequency = freqSmoother.getNextValue();
            params.quality = qualitySmoother.getNextValue();
            if constexpr (! IsCutParameter<ParamType>::value)
            {
                params.gain = gainSmoother.getNextValue().getGain();
            }

            coefficientsGenerator.changeParameters (params);
        }
    }

    void performPreloopUpdate (const ParamType& params)
    {
        updateParams (params);
        updateSmootherTargets();
    }

    void performInnerLoopFilterUpdate (bool onRealTimeThread, int numSamplesToSkip)
    {
        if (currentParams.bypassed)
        {
            return;
        }

        generateNewCoefficientsIfNeeded();
        loadCoefficients (onRealTimeThread);
        advanceSmoothers (numSamplesToSkip);
        checkIfStillSmoothing();
    }

    void initialize (const ParamType& params, float rampTime, bool onRealTimeThread, double sr)
    {
        sampleRate = sr;
        updateParams (params);
        resetSmoothers (rampTime);
        loadCoefficients (onRealTimeThread);
    }

private:
    static const size_t FIFO_SIZE = 2000;

    void discardOldCoefficientsIfAny()
    {
        while (coefficientsFifo.getNumAvailableForReading() > 1)
        {
            FifoDataType unusedCoefficients;
            if (coefficientsFifo.pull (unusedCoefficients))
            {
                if constexpr (IsCutFilter<FilterType>::value)
                {
                    for (auto& coefficient : unusedCoefficients)
                    {
                        coefficientsReleasePool.add (coefficient);
                    }
                }
                else if constexpr (IsParametricFilter<FilterType>::value)
                {
                    coefficientsReleasePool.add (*unusedCoefficients);
                }
                else
                {
                    jassertfalse; //unknown filter type
                }
            }
            else
            {
                jassertfalse; // coefficientsFifo is inconsistent
            }
        }
    }

    FifoDataType loadCoefficientsFromFifo()
    {
    }

    void updateFilterState (CoefficientsPtr& oldState, CoefficientsPtr newState)
    {
        coefficientsReleasePool.add (*newState);
        *oldState = *newState;
    }

    void configureCutFilterChain (const FifoDataType& coefficients)
    {
        filter.template setBypassed<0> (true);
        filter.template setBypassed<1> (true);
        filter.template setBypassed<2> (true);
        filter.template setBypassed<3> (true);

        switch (coefficients.size())
        {
            case 4:
            {
                updateFilterState (filter.template get<3>().coefficients, coefficients[3]);
                filter.template setBypassed<3> (false);
            }
            case 3:
            {
                updateFilterState (filter.template get<2>().coefficients, coefficients[2]);
                filter.template setBypassed<2> (false);
            }
            case 2:
            {
                updateFilterState (filter.template get<1>().coefficients, coefficients[1]);
                filter.template setBypassed<1> (false);
            }
            case 1:
            {
                updateFilterState (filter.template get<0>().coefficients, coefficients[0]);
                filter.template setBypassed<0> (false);
            }
        }
    }

    FilterType filter;
    ParamType currentParams;
    Fifo<FifoDataType, FIFO_SIZE> coefficientsFifo;
    FilterCoefficientGenerator<FifoDataType, ParamType, FunctionType, FIFO_SIZE> coefficientsGenerator { coefficientsFifo };
    ReleasePool<Coefficients> coefficientsReleasePool;

    juce::SmoothedValue<float> freqSmoother;
    juce::SmoothedValue<float> qualitySmoother;
    juce::SmoothedValue<Decibel<float>> gainSmoother;

    juce::Atomic<bool> shouldComputeNewCoefficients { false };
    double sampleRate;
};
