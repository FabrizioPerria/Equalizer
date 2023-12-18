#include "utils/FilterParam.h"

static std::map<FilterInfo::FilterParam, juce::String> filterParamNames = {
    { FilterInfo::FilterParam::GAIN, "gain" },        { FilterInfo::FilterParam::Q, "quality" },
    { FilterInfo::FilterParam::FREQUENCY, "freq" },   { FilterInfo::FilterParam::BYPASS, "bypass" },
    { FilterInfo::FilterParam::FILTER_TYPE, "type" }, { FilterInfo::FilterParam::SLOPE, "slope" }
};

juce::String FilterInfo::getParameterName (int filterNum, int audioChannel, FilterParam param)
{
    auto channelName = audioChannel == 0 ? "L" : "R";
    return "Filter_" + juce::String (filterNum) + "_" + channelName + "_" + filterParamNames[param];
}
