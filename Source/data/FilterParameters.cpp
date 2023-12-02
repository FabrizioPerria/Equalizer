
#include "data/FilterParameters.h"

FilterParametersBase FilterParametersBase::getParameters (const juce::AudioProcessorValueTreeState& apvts,
                                                          int filterIndex,
                                                          double sampleRate)
{
    auto bypassParam =
        apvts.getRawParameterValue (FilterInfo::getParameterName (filterIndex, FilterInfo::FilterParam::BYPASS))->load()
        > 0.5f;
    auto frequencyParam =
        apvts.getRawParameterValue (FilterInfo::getParameterName (filterIndex, FilterInfo::FilterParam::FREQUENCY))
            ->load();
    auto qParam =
        apvts.getRawParameterValue (FilterInfo::getParameterName (filterIndex, FilterInfo::FilterParam::Q))->load();

    return FilterParametersBase { frequencyParam, bypassParam, qParam, sampleRate };
}

FilterParameters FilterParameters::getParameters (const juce::AudioProcessorValueTreeState& apvts,
                                                  int filterIndex,
                                                  FilterInfo::FilterType filterType,
                                                  double sampleRate)
{
    auto baseParams = FilterParametersBase::getParameters (apvts, filterIndex, sampleRate);

    auto gainParam =
        apvts.getRawParameterValue (FilterInfo::getParameterName (filterIndex, FilterInfo::FilterParam::GAIN))->load();

    return FilterParameters { baseParams, filterType, gainParam };
}

HighCutLowCutParameters HighCutLowCutParameters::getParameters (const juce::AudioProcessorValueTreeState& apvts,
                                                                int filterIndex,
                                                                FilterInfo::FilterType filterType,
                                                                double sampleRate)
{
    auto baseParams = FilterParametersBase::getParameters (apvts, filterIndex, sampleRate);
    auto isLowCutParam = filterType == FilterInfo::FilterType::HIGHPASS;
    return HighCutLowCutParameters { baseParams, 1, isLowCutParam };
}

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