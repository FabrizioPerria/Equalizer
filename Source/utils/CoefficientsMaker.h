#pragma once

#include "data/FilterParameters.h"
#include "utils/FilterType.h"
#include <JuceHeader.h>

template <typename FloatType>
struct CoefficientsMaker
{
    CoefficientsMaker() = delete;
    ~CoefficientsMaker() = delete;

    static juce::dsp::IIR::Coefficients<FloatType>::Ptr make (FilterInfo::FilterType type, //
                                                              float freq,
                                                              float q,
                                                              float gain,
                                                              double sampleRate)
    {
        switch (type)
        {
            case FilterInfo::FilterType::FIRST_ORDER_LOWPASS:
                return juce::dsp::IIR::Coefficients<FloatType>::makeFirstOrderLowPass (sampleRate, freq);
            case FilterInfo::FilterType::FIRST_ORDER_HIGHPASS:
                return juce::dsp::IIR::Coefficients<FloatType>::makeFirstOrderHighPass (sampleRate, freq);
            case FilterInfo::FilterType::FIRST_ORDER_ALLPASS:
                return juce::dsp::IIR::Coefficients<FloatType>::makeFirstOrderAllPass (sampleRate, freq);
            case FilterInfo::FilterType::LOWPASS:
                return juce::dsp::IIR::Coefficients<FloatType>::makeLowPass (sampleRate, freq, q);
            case FilterInfo::FilterType::HIGHPASS:
                return juce::dsp::IIR::Coefficients<FloatType>::makeHighPass (sampleRate, freq, q);
            case FilterInfo::FilterType::BANDPASS:
                return juce::dsp::IIR::Coefficients<FloatType>::makeBandPass (sampleRate, freq, q);
            case FilterInfo::FilterType::NOTCH:
                return juce::dsp::IIR::Coefficients<FloatType>::makeNotch (sampleRate, freq, q);
            case FilterInfo::FilterType::ALLPASS:
                return juce::dsp::IIR::Coefficients<FloatType>::makeAllPass (sampleRate, freq, q);
            case FilterInfo::FilterType::LOWSHELF:
                return juce::dsp::IIR::Coefficients<FloatType>::makeLowShelf (sampleRate, freq, q, gain);
            case FilterInfo::FilterType::HIGHSHELF:
                return juce::dsp::IIR::Coefficients<FloatType>::makeHighShelf (sampleRate, freq, q, gain);
            case FilterInfo::FilterType::PEAKFILTER:
                return juce::dsp::IIR::Coefficients<FloatType>::makePeakFilter (sampleRate, freq, q, gain);
            default:
                jassertfalse;
                return nullptr;
        }
    }

    static juce::dsp::IIR::Coefficients<FloatType>::Ptr make (const FilterParameters& params, double sampleRate)
    {
        return make (params.type, params.frequency, params.quality, params.gain, sampleRate);
    }

    static juce::dsp::IIR::Coefficients<FloatType>::Ptr make (const HighCutLowCutParameters& params, double sampleRate)
    {
        using FilterDesign = juce::dsp::FilterDesign<FloatType>;
        if (params.isLowCut)
            return FilterDesign::designIIRHighpassHighOrderButterworthMethod (params.frequency, sampleRate, params.order).getFirst();

        return FilterDesign::designIIRLowpassHighOrderButterworthMethod (params.frequency, sampleRate, params.order).getFirst();
    }
};
