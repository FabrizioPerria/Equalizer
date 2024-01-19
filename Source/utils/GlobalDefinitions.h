#pragma once

/* #define USE_TEST_SIGNAL true */

#if USE_TEST_SIGNAL
#include "utils/FFTDataGenerator.h"
#include <cstddef>

float GetTestSignalFrequency (size_t binNum, FFTOrder fftOrder, double sampleRate);

#endif
