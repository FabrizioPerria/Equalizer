#pragma once

#include "utils/FilterType.h"
#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "utils/FilterParam.h"

struct FilterParametersBase
{
    float frequency = 440.0f;
    bool bypassed = false;
    float quality = 1.0f;
    double sampleRate = 44100.0;

    bool operator== (const FilterParametersBase& other) const
    {
        return juce::approximatelyEqual (frequency, other.frequency) && bypassed == other.bypassed
               && juce::approximatelyEqual (quality, other.quality)
               && juce::approximatelyEqual (sampleRate, other.sampleRate);
    }

    bool operator!= (const FilterParametersBase& other) const
    {
        return ! (*this == other);
    }

    static FilterParametersBase
        getFilterParametersBase (juce::AudioProcessorValueTreeState& apvts, int filterIndex, double sampleRate)
    {
        auto bypassParam =
            apvts.getRawParameterValue (FilterInfo::getParameterName (filterIndex, FilterInfo::FilterParam::BYPASS))
                ->load()
            > 0.5f;
        auto frequencyParam =
            apvts.getRawParameterValue (FilterInfo::getParameterName (filterIndex, FilterInfo::FilterParam::FREQUENCY))
                ->load();
        auto qParam =
            apvts.getRawParameterValue (FilterInfo::getParameterName (filterIndex, FilterInfo::FilterParam::Q))->load();

        return FilterParametersBase { frequencyParam, bypassParam, qParam, sampleRate };
    }
};

struct FilterParameters : public FilterParametersBase
{
    FilterInfo::FilterType type = FilterInfo::FilterType::ALLPASS;
    float gain = 0.0f;

    bool operator== (const FilterParameters& other) const
    {
        return type == other.type && juce::approximatelyEqual (gain, other.gain)
               && static_cast<const FilterParametersBase&> (*this) == static_cast<const FilterParametersBase&> (other);
    }

    bool operator!= (const FilterParameters& other) const
    {
        return ! (*this == other);
    }
};

struct HighCutLowCutParameters : public FilterParametersBase
{
    int order = 1;
    bool isLowCut = false;

    bool operator== (const HighCutLowCutParameters& other) const
    {
        return order == other.order && isLowCut == other.isLowCut
               && static_cast<const FilterParametersBase&> (*this) == static_cast<const FilterParametersBase&> (other);
    }

    bool operator!= (const HighCutLowCutParameters& other) const
    {
        return ! (*this == other);
    }
};

bool needsParametricParams (FilterInfo::FilterType type)
{
    switch (type)
    {
        case FilterInfo::FilterType::FIRST_ORDER_LOWPASS:
        case FilterInfo::FilterType::FIRST_ORDER_HIGHPASS:
        case FilterInfo::FilterType::FIRST_ORDER_ALLPASS:
        case FilterInfo::FilterType::BANDPASS:
        case FilterInfo::FilterType::NOTCH:
        case FilterInfo::FilterType::LOWSHELF:
        case FilterInfo::FilterType::HIGHSHELF:
        case FilterInfo::FilterType::ALLPASS:
        case FilterInfo::FilterType::PEAKFILTER:
            return true;
        case FilterInfo::FilterType::LOWPASS:
        case FilterInfo::FilterType::HIGHPASS:
            return false;
        default:
            jassertfalse;
            return false;
    }
}