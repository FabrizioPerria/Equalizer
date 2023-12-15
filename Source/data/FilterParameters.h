#pragma once

#include "utils/Decibel.h"
#include "utils/FilterType.h"
#include <JuceHeader.h>

struct FilterParametersBase
{
    float frequency = 440.0f;
    bool bypassed = false;
    float quality = 1.0f;
    double sampleRate = 44100.0;
};

struct FilterParameters : FilterParametersBase
{
    FilterInfo::FilterType type = FilterInfo::FilterType::ALLPASS;
    Decibel<float> gain { 0.0f };
};

struct HighCutLowCutParameters : public FilterParametersBase
{
    int order = 1;
    bool isLowCut = false;
};

bool operator== (const FilterParametersBase& lhs, const FilterParametersBase& rhs);
bool operator!= (const FilterParametersBase& lhs, const FilterParametersBase& rhs);

bool operator== (const FilterParameters& lhs, const FilterParameters& rhs);
bool operator!= (const FilterParameters& lhs, const FilterParameters& rhs);

bool operator== (const HighCutLowCutParameters& lhs, const HighCutLowCutParameters& rhs);
bool operator!= (const HighCutLowCutParameters& lhs, const HighCutLowCutParameters& rhs);
