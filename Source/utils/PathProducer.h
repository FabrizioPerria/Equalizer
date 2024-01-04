#pragma once

#include "utils/AnalyzerPathGenerator.h"
#include "utils/FFTDataGenerator.h"
#include "utils/MeterConstants.h"
#include "utils/SingleChannelSampleFifo.h"
#include <JuceHeader.h>

template <typename BlockType>
struct PathProducer : juce::Thread
{
    PathProducer (double sr, SingleChannelSampleFifo<BlockType>&);
    ~PathProducer() override;

    void run() override;
    void changeOrder (FFTOrder o);
    int getFFTSize() const;
    double getBinWidth() const;
    void pauseThread();
    void setFFTRectBounds (juce::Rectangle<float>);

    void setDecayRate (float dr);
    bool pull (juce::Path&&);
    int getNumAvailableForReading() const;
    void toggleProcessing (bool);
    void changePathRange (float negativeInfinityDb, float maxDb);
    void changeSampleRate (double sr);

private:
    SingleChannelSampleFifo<BlockType>* singleChannelSampleFifo;
    FFTDataGenerator fftDataGenerator;
    AnalyzerPathGenerator pathGenerator;

    std::vector<float> renderData;

    void updateRenderData (std::vector<float>& renderData, const std::vector<float>& fftData, int numBins, float decayRate);

    BlockType bufferForGenerator;

    std::atomic<double> sampleRate;
    juce::Rectangle<float> fftBounds;
    /*
     This must be atomic because it's used in inside `run()` as well as 'setDecayRate()' which can be called from any thread.
     */
    std::atomic<float> decayRateInDbPerSec { 0.f }, negativeInfinity { NEGATIVE_INFINITY }, maxDecibels { MAX_DECIBELS };
    /*
     this flag is toggled by the AnalyzerControls (indirectly) on/off button
     and causes processing to be skipped when it's false.
     */
    std::atomic<bool> processingIsEnabled { true };
};