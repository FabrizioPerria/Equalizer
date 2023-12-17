#pragma once

#include <JuceHeader.h>

namespace EqCutFilterDesign
{
using namespace juce;
using namespace juce::dsp;

template <typename FloatType>
juce::ReferenceCountedArray<IIR::Coefficients<FloatType>> designIIRHighpassHighOrderButterworthMethod (FloatType frequency, //
                                                                                                       double sampleRate,
                                                                                                       int order,
                                                                                                       float quality)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= sampleRate * 0.5);
    jassert (order > 0);

    ReferenceCountedArray<IIR::Coefficients<FloatType>> arrayFilters;

    auto numBiquadFilters = order / 2;
    auto a = std::pow (quality * MathConstants<float>::sqrt2, 1.0f / static_cast<float> (numBiquadFilters));
    if (order % 2 == 1)
    {
        arrayFilters.add (*IIR::Coefficients<FloatType>::makeFirstOrderHighPass (sampleRate, frequency));

        for (int i = 0; i < numBiquadFilters; ++i)
        {
            auto Q = a / (2.0 * std::cos ((i + 1.0) * MathConstants<double>::pi / order));
            arrayFilters.add (*IIR::Coefficients<FloatType>::makeHighPass (sampleRate, frequency, static_cast<FloatType> (Q)));
        }
    }
    else
    {
        for (int i = 0; i < numBiquadFilters; ++i)
        {
            auto Q = a / (2.0 * std::cos ((2.0 * i + 1.0) * MathConstants<double>::pi / (order * 2.0)));
            arrayFilters.add (*IIR::Coefficients<FloatType>::makeHighPass (sampleRate, frequency, static_cast<FloatType> (Q)));
        }
    }

    return arrayFilters;
}

template <typename FloatType>
ReferenceCountedArray<IIR::Coefficients<FloatType>> designIIRLowpassHighOrderButterworthMethod (FloatType frequency, //
                                                                                                double sampleRate,
                                                                                                int order,
                                                                                                float quality)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= sampleRate * 0.5);
    jassert (order > 0);

    ReferenceCountedArray<IIR::Coefficients<FloatType>> arrayFilters;

    auto numBiquadFilters = order / 2;
    auto a = std::pow (quality * MathConstants<float>::sqrt2, 1.0f / static_cast<float> (numBiquadFilters));
    if (order % 2 == 1)
    {
        arrayFilters.add (*IIR::Coefficients<FloatType>::makeFirstOrderLowPass (sampleRate, frequency));

        for (int i = 0; i < numBiquadFilters; ++i)
        {
            auto Q = a / (2.0 * std::cos ((i + 1.0) * MathConstants<double>::pi / order));
            arrayFilters.add (*IIR::Coefficients<FloatType>::makeLowPass (sampleRate, frequency, static_cast<FloatType> (Q)));
        }
    }
    else
    {
        for (int i = 0; i < numBiquadFilters; ++i)
        {
            auto Q = a / (2.0 * std::cos ((2.0 * i + 1.0) * MathConstants<double>::pi / (order * 2.0)));
            arrayFilters.add (*IIR::Coefficients<FloatType>::makeLowPass (sampleRate, frequency, static_cast<FloatType> (Q)));
        }
    }

    return arrayFilters;
}

} // namespace EqCutFilterDesign
