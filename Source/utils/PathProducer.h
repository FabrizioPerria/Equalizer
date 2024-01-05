#pragma once

#include "utils/AnalyzerPathGenerator.h"
#include "utils/FFTDataGenerator.h"
#include "utils/MeterConstants.h"
#include "utils/SingleChannelSampleFifo.h"
#include <JuceHeader.h>

template <typename BlockType>
struct PathProducer : juce::Thread
{
    PathProducer (double sr, SingleChannelSampleFifo<BlockType>& fifoRef)
        : Thread ("PathProducer"), singleChannelSampleFifo { &fifoRef }, sampleRate { sr }
    {
    }

    ~PathProducer() override
    {
        stopThread (1000);
    }

    void run() override
    {
        BlockType bufferToFill;
        while (! threadShouldExit())
        {
            if (! processingIsEnabled)
            {
                wait (LOOP_DELAY);
                continue;
            }
            auto fftSize = getFFTSize();
            auto bufferForGeneratorSize = bufferForGenerator.getNumSamples();
            BlockType tmpBuffer (1, bufferForGeneratorSize);

            while (! threadShouldExit() && singleChannelSampleFifo->getNumCompleteBuffersAvailable() > 0)
            {
                auto success = singleChannelSampleFifo->getAudioBuffer (bufferToFill);
                jassert (success);
                auto bufferToFillSize = bufferToFill.getNumSamples();

                tmpBuffer.copyFrom (0, 0, bufferForGenerator, 0, bufferToFillSize, bufferForGeneratorSize - bufferToFillSize);
                tmpBuffer.copyFrom (0, bufferForGeneratorSize - bufferToFillSize, bufferToFill, 0, 0, bufferToFillSize);
                std::swap (bufferForGenerator, tmpBuffer);
                fftDataGenerator.produceFFTDataForRendering (bufferForGenerator);
            }

            while (! threadShouldExit() && fftDataGenerator.getNumAvailableFFTDataBlocks() > 0)
            {
                std::vector<float> fftData;
                auto success = fftDataGenerator.getFFTData (std::move (fftData));
                jassert (success);
                updateRenderData (renderData, fftData, fftSize / 2, static_cast<float> (LOOP_DELAY) * decayRateInDbPerSec.load() / 1000.f);
                pathGenerator.generatePath (renderData, fftBounds, fftSize, getBinWidth());
            }
            wait (LOOP_DELAY);
        }
    }

    void changeOrder (FFTOrder o)
    {
        pauseThread();
        fftDataGenerator.changeOrder (o);
        renderData.clear();
        auto fftSize = getFFTSize();
        renderData.resize (fftSize / 2 + 1, negativeInfinity.load());

        bufferForGenerator.setSize (1, fftSize, false, false, true);
        bufferForGenerator.clear();

        while (! singleChannelSampleFifo->isPrepared())
        {
            wait (LOOP_DELAY);
        }
        if (! fftBounds.isEmpty())
        {
            startThread();
        }
    }

    int getFFTSize() const
    {
        return static_cast<int>(fftDataGenerator.getFFTSize());
    }

    double getBinWidth() const
    {
        return sampleRate.load() / (double) getFFTSize();
    }

    void pauseThread()
    {
        auto didStop = stopThread (500);
        jassert (didStop);
    }

    void setFFTRectBounds (juce::Rectangle<float> bounds)
    {
        pauseThread();
        if (bounds.isEmpty())
        {
            return;
        }
        fftBounds = bounds;
        startThread();
    }

    void setDecayRate (float dr)
    {
        // eventually check if the decay rate is within a certain range minDbperSec - maxDbPerSec
        decayRateInDbPerSec = dr;
    }

    bool pull (juce::Path& path)
    {
        //TODO: use tracer to see if move semantics is faster than copy semantics
        return pathGenerator.getPath (std::move (path));
    }

    int getNumAvailableForReading() const
    {
        return pathGenerator.getNumPathsAvailable();
    }

    void toggleProcessing (bool enabled)
    {
        processingIsEnabled = enabled;
    }

    void changePathRange (float negativeInfinityDb, float maxDb)
    {
        negativeInfinity = negativeInfinityDb;
        maxDecibels = maxDb;
    }

    void changeSampleRate (double sr)
    {
        pauseThread();
        sampleRate = sr;
        startThread();
    }

private:
    SingleChannelSampleFifo<BlockType>* singleChannelSampleFifo;
    FFTDataGenerator fftDataGenerator;
    AnalyzerPathGenerator pathGenerator;

    std::vector<float> renderData;

    void updateRenderData (std::vector<float>& renderDataToUpdate, const std::vector<float>& fftData, int numBins, float decayRate)
    {
        if (decayRate >= 0.0f)
        {
            for (size_t bin = 0; bin < static_cast<size_t>(numBins); ++bin)
            {
                auto previous = renderDataToUpdate[bin];
                auto candidate = fftData[bin];
                auto finalValue = juce::jmax (candidate, previous - decayRate);
                renderDataToUpdate[bin] = juce::jlimit (negativeInfinity.load(), maxDecibels.load(), finalValue);
            }
        }
    }

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

    const int LOOP_DELAY { 10 };
};
