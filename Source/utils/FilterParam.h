#pragma once

#include <JuceHeader.h>
#include <utils/EqParam.h>

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

juce::String getParameterName (int filterNum, Channel audioChannel, FilterParam param);
} // namespace FilterInfo
