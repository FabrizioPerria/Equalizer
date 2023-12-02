#include "FilterType.h"
#include "utils/FilterParam.h"

static std::map<FilterInfo::FilterType, juce::String> filterTypeMap = {
    { FilterInfo::FilterType::FIRST_ORDER_LOWPASS, "First Order Lowpass" },
    { FilterInfo::FilterType::FIRST_ORDER_HIGHPASS, "First Order Highpass" },
    { FilterInfo::FilterType::FIRST_ORDER_ALLPASS, "First Order Allpass" },
    { FilterInfo::FilterType::LOWPASS, "Lowpass" },
    { FilterInfo::FilterType::HIGHPASS, "Highpass" },
    { FilterInfo::FilterType::BANDPASS, "Bandpass" },
    { FilterInfo::FilterType::NOTCH, "Notch" },
    { FilterInfo::FilterType::ALLPASS, "Allpass" },
    { FilterInfo::FilterType::LOWSHELF, "Lowshelf" },
    { FilterInfo::FilterType::HIGHSHELF, "Highshelf" },
    { FilterInfo::FilterType::PEAKFILTER, "Peakfilter" },
};

FilterInfo::FilterType FilterInfo::getFilterType (const juce::AudioProcessorValueTreeState& apvts, int filterIndex)
{
    auto filterTypeParam =
        apvts.getRawParameterValue (FilterInfo::getParameterName (filterIndex, FilterInfo::FilterParam::FILTER_TYPE));
    return static_cast<FilterInfo::FilterType> (filterTypeParam->load());
}

juce::String FilterInfo::getFilterTypeName (FilterType filterType)
{
    auto it = filterTypeMap.find (filterType);
    if (it != filterTypeMap.end())
        return it->second;
    return "Unknown";
}

juce::StringArray FilterInfo::getFilterTypeNames()
{
    juce::StringArray names;
    for (auto& it : filterTypeMap)
        names.add (it.second);
    return names;
}
