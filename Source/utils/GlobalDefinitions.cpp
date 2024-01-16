#include "utils/GlobalDefinitions.h"

#if USE_TEST_SIGNAL
#include "utils/FFTDataGenerator.h"

#include <JuceHeader.h>
#include <cstddef>
float GetTestSignalFrequency (size_t binNum, FFTOrder fftOrder, double sampleRate)
{
    size_t fftSize = 1 << static_cast<int> (fftOrder);
    auto binWidth = sampleRate / fftSize;

    size_t numBins = fftSize / 2 + 1;
    jassert (binNum < numBins);

    return static_cast<float> (binNum * binWidth);
}
#endif
