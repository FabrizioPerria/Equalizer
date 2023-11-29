#pragma once

#include <juce_core/juce_core.h>

namespace FilterInfo
{
    enum class FilterParam
    {
        GAIN,
        Q,
        FREQUENCY,
        BYPASS,
        FILTER_TYPE,
    };

    juce::String getParameterName(int filterNum, FilterParam param);
}
