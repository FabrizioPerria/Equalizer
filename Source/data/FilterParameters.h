#pragma once

#include "data/FilterParametersBase.h"
#include "utils/FilterType.h"

struct FilterParameters : public FilterParametersBase
{
    FilterInfo::FilterType type = FilterInfo::FilterType::ALLPASS;
    float gain = 0.0f;
};
