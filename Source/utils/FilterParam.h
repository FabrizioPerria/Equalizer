#pragma once

#include <JuceHeader.h>

namespace FilterInfo
{
    enum class FilterParam
    {
        GAIN,
        Q,
        FREQUENCY,
        BYPASS,
        FILTER_TYPE,
        SLOPE
    };

    juce::String getParameterName(int filterNum, FilterParam param);
}
