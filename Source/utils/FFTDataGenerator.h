#pragma once

#include "utils/Fifo.h"
#include <JuceHeader.h>

enum class FFTOrder
{
    order2048 = 11,
    order4096 = 12,
    order8192 = 13
};

struct FFTDataGenerator
{
    /*
     produces the FFT data from an audio buffer.
     */
    void produceFFTDataForRendering (const juce::AudioBuffer<float>& audioData);

    void changeOrder (FFTOrder newOrder);

    size_t getFFTSize() const;

    int getNumAvailableFFTDataBlocks() const;

    bool getFFTData (std::vector<float>&& fftDataReceiver);

private:
    FFTOrder order { FFTOrder::order2048 };
    std::vector<float> fftData;
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;

    Fifo<std::vector<float>, 100> fftDataFifo;
};
