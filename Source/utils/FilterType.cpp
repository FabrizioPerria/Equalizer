#include "FilterType.h"

static std::map<FilterInfo::FilterType, juce::String> filterTypeMap = {
	{FilterInfo::FilterType::FIRST_ORDER_LOWPASS, "First Order Lowpass"},
	{FilterInfo::FilterType::FIRST_ORDER_HIGHPASS, "First Order Highpass"},
	{FilterInfo::FilterType::FIRST_ORDER_ALLPASS, "First Order Allpass"},
	{FilterInfo::FilterType::LOWPASS, "Lowpass"},
	{FilterInfo::FilterType::HIGHPASS, "Highpass"},
	{FilterInfo::FilterType::BANDPASS, "Bandpass"},
	{FilterInfo::FilterType::NOTCH, "Notch"},
	{FilterInfo::FilterType::ALLPASS, "Allpass"},
	{FilterInfo::FilterType::LOWSHELF, "Lowshelf"},
	{FilterInfo::FilterType::HIGHSHELF, "Highshelf"},
	{FilterInfo::FilterType::PEAKFILTER, "Peakfilter"},
};

juce::String FilterInfo::getFilterTypeName(FilterType filterType)
{
    auto it = filterTypeMap.find(filterType);
    if (it != filterTypeMap.end())
        return it->second;
    return "Unknown";
}

juce::StringArray FilterInfo::getFilterTypeNames()
{
    juce::StringArray names;
    for (auto& it : filterTypeMap)
        names.add(it.second);
    return names;
}
