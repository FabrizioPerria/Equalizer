#pragma once

#include "utils/FilterParam.h"
#include "utils/FilterType.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>

struct FilterParametersBase
{
    float frequency = 440.0f;
    bool bypassed = false;
    float quality = 1.0f;
    double sampleRate = 44100.0;

    static FilterParametersBase getParameters (const juce::AudioProcessorValueTreeState& apvts, //
                                               int filterIndex,
                                               double sampleRate);

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
};

struct FilterParameters : FilterParametersBase
{
    FilterInfo::FilterType type = FilterInfo::FilterType::ALLPASS;
    float gain = 0.0f;

    static FilterParameters getParameters (const juce::AudioProcessorValueTreeState& apvts, //
                                           int filterIndex,
                                           FilterInfo::FilterType filterType,
                                           double sampleRate);

    bool operator== (const FilterParameters& other) const
    {
        return type == other.type //
               && juce::approximatelyEqual (gain, other.gain)
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

    static HighCutLowCutParameters getParameters (const juce::AudioProcessorValueTreeState& apvts, //
                                                  int filterIndex,
                                                  FilterInfo::FilterType filterType,
                                                  double sampleRate);

    bool operator== (const HighCutLowCutParameters& other) const
    {
        return order == other.order //
               && isLowCut == other.isLowCut
               && static_cast<const FilterParametersBase&> (*this) == static_cast<const FilterParametersBase&> (other);
    }

    bool operator!= (const HighCutLowCutParameters& other) const
    {
        return ! (*this == other);
    }
};

bool needsParametricParams (FilterInfo::FilterType type);