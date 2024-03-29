#include "utils/FFTDataGenerator.h"
#include "utils/MeterConstants.h"

void FFTDataGenerator::produceFFTDataForRendering (const juce::AudioBuffer<float>& audioData)
{
    auto fftSize = getFFTSize();

    fftData.resize (2 * fftSize, 0);

    auto reader = audioData.getReadPointer (0);
    std::copy (reader, reader + fftSize, fftData.begin());

    window->multiplyWithWindowingTable (fftData.data(), fftSize);

    forwardFFT->performFrequencyOnlyForwardTransform (fftData.data(), true);

    auto numBins = fftSize / 2;
    juce::FloatVectorOperations::multiply (fftData.data(), 1.0f / static_cast<float> (numBins), numBins + 1);

    for (size_t i = 0; i < numBins; ++i)
    {
        fftData[i] = juce::Decibels::gainToDecibels (fftData[i], NEGATIVE_INFINITY);
    }

    fftDataFifo.push (fftData);
}

void FFTDataGenerator::changeOrder (FFTOrder newOrder)
{
    order = newOrder;
    auto fftSize = getFFTSize();
    window = std::make_unique<juce::dsp::WindowingFunction<float>> (fftSize, juce::dsp::WindowingFunction<float>::blackmanHarris);
    forwardFFT = std::make_unique<juce::dsp::FFT> (static_cast<int> (order));
    fftData.clear();
    fftData.resize (2 * fftSize, 0);
    fftDataFifo.prepare (2 * fftSize);
}

size_t FFTDataGenerator::getFFTSize() const
{
    return 1 << static_cast<int> (order);
}

int FFTDataGenerator::getNumAvailableFFTDataBlocks() const
{
    return fftDataFifo.getNumAvailableForReading();
}

bool FFTDataGenerator::getFFTData (std::vector<float>&& fftDataReceiver)
{
    return fftDataFifo.exchange (std::move (fftDataReceiver));
}
