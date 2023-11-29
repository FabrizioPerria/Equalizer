#pragma once

#include "utils/FilterType.h"

struct FilterParametersBase
{
    float frequency = 440.0f;
    bool bypassed = false;
    float quality = 1.0f;
    double sampleRate = 44100.0;
};

struct FilterParameters : public FilterParametersBase
{
    FilterInfo::FilterType type = FilterInfo::FilterType::ALLPASS;
    float gain = 0.0f;
};

struct HighCutLowCutParameters : public FilterParametersBase
{
    int order = 1;
    bool isLowCut = false;
};

