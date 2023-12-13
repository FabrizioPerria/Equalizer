/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "data/FilterParameters.h"
#include "utils/CoefficientsMaker.h"
#include "utils/Fifo.h"
#include "utils/FilterCoefficientGenerator.h"
#include "utils/FilterType.h"
#include "utils/ReleasePool.h"
#include <JuceHeader.h>

//==============================================================================
enum class ChainPositions
{
    LOWCUT,
    PARAMETRIC_FILTER,
    HIGHCUT,
    NUM_FILTERS
};

// ====================================================================================================
enum class Slope
{
    SLOPE_6,
    SLOPE_12,
    SLOPE_18,
    SLOPE_24,
    SLOPE_30,
    SLOPE_36,
    SLOPE_40,
    SLOPE_48
};
// ====================================================================================================
class EqualizerAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    EqualizerAudioProcessor();
    ~EqualizerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Params", createParameterLayout() };

    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    using CoefficientsPtr = Coefficients::Ptr;
    using CutCoefficients = juce::ReferenceCountedArray<Coefficients>;
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    using MonoFilter = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

private:
    const static size_t FIFO_SIZE = 20;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    static const std::map<FilterInfo::FilterType, juce::String> filterTypeMap;
    FilterInfo::FilterType getFilterType (int filterIndex);
    static juce::StringArray getFilterTypeNames();
    static juce::StringArray getSlopeNames();

    float getRawParameter (int filterIndex, FilterInfo::FilterParam filterParameter);

    FilterParametersBase getBaseParameters (int filterIndex);
    FilterParameters getParametricParameters (int filterIndex, FilterInfo::FilterType filterType);
    HighCutLowCutParameters getCutParameters (int filterIndex, FilterInfo::FilterType filterType);

    void updateCoefficients (CoefficientsPtr& oldCoefficients, const CoefficientsPtr& newCoefficients, ReleasePool<Coefficients>& pool);

    template <int Index, typename ChainType>
    void update (ChainType& chain, CutCoefficients& coefficients, ReleasePool<Coefficients>& pool)
    {
        updateCoefficients (chain.template get<Index>().coefficients, coefficients[Index], pool);
        chain.template setBypassed<Index> (false);
    }

    template <typename ChainType>
    void updateCutFilter (ChainType& chain, CutCoefficients& coefficients, ReleasePool<Coefficients>& pool)
    {
        chain.template setBypassed<0> (true);
        chain.template setBypassed<1> (true);
        chain.template setBypassed<2> (true);
        chain.template setBypassed<3> (true);

        switch (coefficients.size())
        {
            case 4:
            {
                update<3> (chain, coefficients, pool);
            }
            case 3:
            {
                update<2> (chain, coefficients, pool);
            }
            case 2:
            {
                update<1> (chain, coefficients, pool);
            }
            case 1:
            {
                update<0> (chain, coefficients, pool);
            }
        }
    }

    template <typename CoefficientType>
    CoefficientType fetchCoefficientsFromFifo (Fifo<CoefficientType, FIFO_SIZE>& fifo, ReleasePool<Coefficients>& pool)
    {
        while (fifo.getNumAvailableForReading() > 1)
        {
            CoefficientType unusedCoefficients;
            if (fifo.pull (unusedCoefficients))
            {
                if constexpr (std::is_same_v<CoefficientType, CutCoefficients>)
                {
                    for (auto& coefficient : unusedCoefficients)
                    {
                        pool.add (coefficient);
                    }
                }
                else if constexpr (std::is_same_v<CoefficientType, CoefficientsPtr>)
                {
                    pool.add (*unusedCoefficients);
                }
                else
                {
                    jassertfalse; //unknown coefficient type
                }
            }
            else
            {
                jassertfalse; // fifo is inconsistent
            }
        }
        CoefficientType coefficients;
        auto success = fifo.pull (coefficients);
        jassert (success);
        return coefficients;
    }

    template <ChainPositions ChainPosition, typename ParamsType, typename CoefficientsGenerator>
    void updateFilter (ParamsType& oldParams, const ParamsType& newParams, CoefficientsGenerator& generator)
    {
        const int ChainPositionInt = static_cast<int> (ChainPosition);
        leftChain.setBypassed<ChainPositionInt> (newParams.bypassed);
        rightChain.setBypassed<ChainPositionInt> (newParams.bypassed);
        auto& leftFilter = leftChain.template get<ChainPositionInt>();
        auto& rightFilter = rightChain.template get<ChainPositionInt>();

        if (newParams != oldParams)
        {
            generator.changeParameters (newParams);
            oldParams = newParams;
        }

        const bool isHighCutFilter = ChainPosition == ChainPositions::HIGHCUT;
        const bool isLowCutFilter = ChainPosition == ChainPositions::LOWCUT;
        if constexpr (isHighCutFilter || isLowCutFilter)
        {
            CutCoefficients coefficients;

            auto& releasePoolToUse = isHighCutFilter ? highCutReleasePool : lowCutReleasePool;
            auto& fifoToUse = isHighCutFilter ? highcutFilterFifo : lowcutFilterFifo;

            if (fifoToUse.getNumAvailableForReading() > 0)
            {
                coefficients = fetchCoefficientsFromFifo<CutCoefficients> (fifoToUse, releasePoolToUse);

                updateCutFilter (leftFilter, coefficients, releasePoolToUse);
                updateCutFilter (rightFilter, coefficients, releasePoolToUse);
            }
        }
        else
        {
            CoefficientsPtr coefficients;

            if (parametricFilterFifo.getNumAvailableForReading() > 0)
            {
                coefficients = fetchCoefficientsFromFifo (parametricFilterFifo, parametricReleasePool);

                updateCoefficients (leftFilter.coefficients, coefficients, parametricReleasePool);
                updateCoefficients (rightFilter.coefficients, coefficients, parametricReleasePool);
            }
        }
    }

    void updateFilters();

    std::array<FilterParameters, static_cast<size_t> (ChainPositions::NUM_FILTERS)> oldFilterParams;
    std::array<HighCutLowCutParameters, static_cast<size_t> (ChainPositions::NUM_FILTERS)> oldHighCutLowCutParams;

    MonoFilter leftChain, rightChain;

    Fifo<CutCoefficients, FIFO_SIZE> lowcutFilterFifo;
    Fifo<CoefficientsPtr, FIFO_SIZE> parametricFilterFifo;
    Fifo<CutCoefficients, FIFO_SIZE> highcutFilterFifo;

    using CutCoefficientGenerator = FilterCoefficientGenerator<CutCoefficients,
                                                               HighCutLowCutParameters,
                                                               CoefficientsMaker<float>,
                                                               FIFO_SIZE>;

    CutCoefficientGenerator lowCutCoefficientsGenerator { lowcutFilterFifo };
    CutCoefficientGenerator highCutCoefficientsGenerator { highcutFilterFifo };

    using ParametricCoefficientGenerator = FilterCoefficientGenerator<CoefficientsPtr, //
                                                                      FilterParameters,
                                                                      CoefficientsMaker<float>,
                                                                      FIFO_SIZE>;

    ParametricCoefficientGenerator parametricCoefficientsGenerator { parametricFilterFifo };

    ReleasePool<Coefficients> lowCutReleasePool;
    ReleasePool<Coefficients> parametricReleasePool;
    ReleasePool<Coefficients> highCutReleasePool;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessor)
};
