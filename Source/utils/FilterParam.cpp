#include "utils/FilterParam.h"
#include "utils/EqParam.h"

static std::map<FilterInfo::FilterParam, juce::String> filterParamNames = {
    { FilterInfo::FilterParam::GAIN, "gain" },        { FilterInfo::FilterParam::Q, "quality" },
    { FilterInfo::FilterParam::FREQUENCY, "freq" },   { FilterInfo::FilterParam::BYPASS, "bypass" },
    { FilterInfo::FilterParam::FILTER_TYPE, "type" }, { FilterInfo::FilterParam::SLOPE, "slope" }
};

juce::String FilterInfo::getParameterName (int filterNum, Channel audioChannel, FilterParam param)
{
    auto channelName = audioChannel == Channel::LEFT ? "L" : "R";
    return "Filter_" + juce::String (filterNum) + "_" + channelName + "_" + filterParamNames[param];
}
