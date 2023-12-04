#include "utils/FilterParam.h"

static std::map<FilterInfo::FilterParam, juce::String> filterParamNames = {
    { FilterInfo::FilterParam::GAIN, "gain" },        { FilterInfo::FilterParam::Q, "slope" },
    { FilterInfo::FilterParam::FREQUENCY, "freq" },   { FilterInfo::FilterParam::BYPASS, "bypass" },
    { FilterInfo::FilterParam::FILTER_TYPE, "type" },
};

juce::String FilterInfo::getParameterName (int filterNum, FilterParam param)
{
    return "Filter_" + juce::String (filterNum) + "_" + filterParamNames[param];
}
