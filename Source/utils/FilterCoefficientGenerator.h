#pragma once

#include "utils/Fifo.h"
#include <JuceHeader.h>

template <typename CoefficientType>
struct IsCoefficientsArray : std::false_type
{
};

template <typename CoefficientType>
struct IsCoefficientsArray<juce::ReferenceCountedArray<CoefficientType>> : std::true_type
{
};

template <typename CoefficientType, typename ParamType, typename MakeFunction, size_t Size>
struct FilterCoefficientGenerator : juce::Thread
{
    FilterCoefficientGenerator (Fifo<CoefficientType, Size>& coefficientsBuffer)
        : Thread { "Filter Coefficient Generator" }, coefficientsFifo (coefficientsBuffer)
    {
        startThread();
    }

    ~FilterCoefficientGenerator() override
    {
        stopThread (1000);
    }

    void changeParameters (ParamType params)
    {
        jassert (parametersFifo.push (params));
        hasParametersChanged = true;
    }

    void run() override
    {
        while (! threadShouldExit())
        {
            if (hasParametersChanged.compareAndSetBool (false, true))
            {
                while (parametersFifo.getNumAvailableForReading() > 0)
                {
                    ParamType poppedParams;
                    jassert (parametersFifo.pull (poppedParams));

                    auto coefficients = MakeFunction::make (poppedParams);
                    if (receivedNewCoefficients (coefficients))
                    {
                        auto pushSuccess = coefficientsFifo.push (coefficients);
                        jassert (pushSuccess);
                    }
                }
            }

            wait (SLEEP_TIME_MS);
        }
    }

private:
    bool receivedNewCoefficients (CoefficientType& coefficients)
    {
        if constexpr (IsCoefficientsArray<CoefficientType>::value)
        {
            return coefficients.size() > 0;
        }
        else
        {
            return coefficients != nullptr;
        }
    }

    static const int SLEEP_TIME_MS = 10;
    Fifo<CoefficientType, Size>& coefficientsFifo;
    Fifo<ParamType, Size> parametersFifo;
    juce::Atomic<bool> hasParametersChanged { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterCoefficientGenerator)
};
