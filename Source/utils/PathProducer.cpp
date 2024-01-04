#include "utils/PathProducer.h"
#include <JuceHeader.h>

template <typename BlockType>
PathProducer<BlockType>::PathProducer (double sr, SingleChannelSampleFifo<BlockType>& scsf)
    : sampleRate (sr), singleChannelSampleFifo (&scsf), Thread ("PathProducer")
{
    startThread();
}

template <typename BlockType>
PathProducer<BlockType>::~PathProducer()
{
    stopThread (1000);
}

template <typename BlockType>
void PathProducer<BlockType>::run()
{
    BlockType bufferToFill;
    while (! threadShouldExit())
    {
        if (! processingIsEnabled)
        {
            wait (100);
            continue;
        }
        auto fftSize = getFFTSize();
        auto bufferForGeneratorSize = bufferForGenerator.getSize();
        auto tmpBuffer = BlockType (1, bufferForGeneratorSize);

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
            updateRenderData (renderData, fftData, fftSize, decayRateInDbPerSec);
            pathGenerator.generatePath (renderData, fftBounds, fftSize, getBinWidth());
        }
        wait (100);
    }
}
template <typename BlockType>
void PathProducer<BlockType>::changeOrder (FFTOrder o)
{
    pauseThread();
    fftDataGenerator.changeOrder (o);
    renderData.clear();
    renderData.resize (getFFTSize() / 2 + 1, negativeInfinity.load());

    bufferForGenerator.setSize (1, getFFTSize());
    bufferForGenerator.clear();

    while (! singleChannelSampleFifo->isPrepared())
    {
        wait (10);
    }
    if (! fftBounds.isEmpty())
    {
        startThread();
    }
}

template <typename BlockType>
int PathProducer<BlockType>::getFFTSize() const
{
    return fftDataGenerator.getFFTSize();
}

template <typename BlockType>
double PathProducer<BlockType>::getBinWidth() const
{
    return sampleRate.load() / (double) getFFTSize();
}

template <typename BlockType>
void PathProducer<BlockType>::pauseThread()
{
    auto didStop = stopThread (1000);
    jassert (didStop);
}

template <typename BlockType>
void PathProducer<BlockType>::setFFTRectBounds (juce::Rectangle<float> bounds)
{
    pauseThread();
    if (bounds.isEmpty())
    {
        return;
    }
    fftBounds = bounds;
    startThread();
}

template <typename BlockType>
void PathProducer<BlockType>::setDecayRate (float dr)
{
    // eventually check if the decay rate is within a certain range minDbperSec - maxDbPerSec
    decayRateInDbPerSec = dr;
}

template <typename BlockType>
bool PathProducer<BlockType>::pull (juce::Path&& path)
{
    //TODO: use tracer to see if move semantics is faster than copy semantics
    pathGenerator.getPath (std::move (path));
}

template <typename BlockType>
int PathProducer<BlockType>::getNumAvailableForReading() const
{
    return pathGenerator.getNumPathsAvailable();
}

template <typename BlockType>
void PathProducer<BlockType>::toggleProcessing (bool enabled)
{
    processingIsEnabled = enabled;
}

template <typename BlockType>
void PathProducer<BlockType>::changePathRange (float negativeInfinityDb, float maxDb)
{
    negativeInfinity = negativeInfinityDb;
    maxDecibels = maxDb;
}

template <typename BlockType>
void PathProducer<BlockType>::changeSampleRate (double sr)
{
    pauseThread();
    sampleRate = sr;
    startThread();
}

template <typename BlockType>
void PathProducer<BlockType>::updateRenderData (std::vector<float>& renderDataToUpdate,
                                                const std::vector<float>& fftData,
                                                int numBins,
                                                float decayRate)
{
    if (decayRate >= 0.0f)
    {
        for (auto bin = 0; bin < numBins; ++bin)
        {
            auto previous = renderDataToUpdate[bin];
            auto candidate = fftData[bin];
            auto finalValue = juce::jmax (candidate, previous - decayRate);
            renderDataToUpdate[bin] = juce::jlimit (negativeInfinity.load(), maxDecibels.load(), finalValue);
        }
    }
}
