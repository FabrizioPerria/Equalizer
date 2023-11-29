#pragma once

#include <juce_core/juce_core.h>

namespace FilterInfo
{
enum class FilterType
{
    FIRST_ORDER_LOWPASS,
    FIRST_ORDER_HIGHPASS,
    FIRST_ORDER_ALLPASS,
    LOWPASS,
    HIGHPASS,
    BANDPASS,
    NOTCH,
    ALLPASS,
    LOWSHELF,
    HIGHSHELF,
    PEAKFILTER,
};

juce::String getFilterTypeName (FilterType filterType);
juce::StringArray getFilterTypeNames();
} // namespace FilterInfo
