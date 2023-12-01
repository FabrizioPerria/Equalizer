#pragma once

#include "data/FilterParameters.h"
#include "utils/FilterType.h"
#include <juce_dsp/juce_dsp.h>

template <typename T>
class CoefficientsMaker
{
public:
    CoefficientsMaker() = delete;
    ~CoefficientsMaker() = delete;

    static juce::dsp::IIR::Coefficients<T>::Ptr make (FilterInfo::FilterType type,
                                                      float freq,
                                                      float q,
                                                      float gain,
                                                      double sampleRate)
    {
        switch (type)
        {
            case FilterInfo::FilterType::FIRST_ORDER_LOWPASS:
                return juce::dsp::IIR::Coefficients<T>::makeFirstOrderLowPass (sampleRate, freq);
            case FilterInfo::FilterType::FIRST_ORDER_HIGHPASS:
                return juce::dsp::IIR::Coefficients<T>::makeFirstOrderHighPass (sampleRate, freq);
            case FilterInfo::FilterType::FIRST_ORDER_ALLPASS:
                return juce::dsp::IIR::Coefficients<T>::makeFirstOrderAllPass (sampleRate, freq);
            case FilterInfo::FilterType::LOWPASS:
                return juce::dsp::IIR::Coefficients<T>::makeLowPass (sampleRate, freq, q);
            case FilterInfo::FilterType::HIGHPASS:
                return juce::dsp::IIR::Coefficients<T>::makeHighPass (sampleRate, freq, q);
            case FilterInfo::FilterType::BANDPASS:
                return juce::dsp::IIR::Coefficients<T>::makeBandPass (sampleRate, freq, q);
            case FilterInfo::FilterType::NOTCH:
                return juce::dsp::IIR::Coefficients<T>::makeNotch (sampleRate, freq, q);
            case FilterInfo::FilterType::ALLPASS:
                return juce::dsp::IIR::Coefficients<T>::makeAllPass (sampleRate, freq, q);
            case FilterInfo::FilterType::LOWSHELF:
                return juce::dsp::IIR::Coefficients<T>::makeLowShelf (sampleRate, freq, q, gain);
            case FilterInfo::FilterType::HIGHSHELF:
                return juce::dsp::IIR::Coefficients<T>::makeHighShelf (sampleRate, freq, q, gain);
            case FilterInfo::FilterType::PEAKFILTER:
                return juce::dsp::IIR::Coefficients<T>::makePeakFilter (sampleRate, freq, q, gain);
            default:
                jassertfalse;
                return nullptr;
        }
    }

    static juce::dsp::IIR::Coefficients<T>::Ptr make (const FilterParameters& params, double sampleRate)
    {
        return make (params.type, params.frequency, params.quality, params.gain, sampleRate);
    }

    static juce::dsp::IIR::Coefficients<T>::Ptr make (const HighCutLowCutParameters& params, double sampleRate)
    {
        if (params.isLowCut)
            return juce::dsp::FilterDesign<T>::designIIRHighpassHighOrderButterworthMethod (
                       params.frequency, sampleRate, params.order)
                .getFirst();

        return juce::dsp::FilterDesign<T>::designIIRLowpassHighOrderButterworthMethod (
                   params.frequency, sampleRate, params.order)
            .getFirst();
    }
};
