#pragma once

#include <JuceHeader.h>

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
} // namespace FilterInfo
