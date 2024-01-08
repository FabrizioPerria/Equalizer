#pragma once

#include "utils/EqParam.h"
#include "utils/Fifo.h"

template <typename BlockType>
struct SingleChannelSampleFifo
{
    using SampleType = typename BlockType::SampleType;
    SingleChannelSampleFifo (Channel ch) : channelToUse (ch)
    {
    }

    void update (const BlockType& buffer)
    {
        if (! isPrepared())
        {
            jassertfalse;
            return;
        }

        if (buffer.getNumChannels() > 0)
        {
            auto* reader = buffer.getReadPointer (static_cast<int> (channelToUse));

            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                pushNextSampleIntoFifo (reader[i]);
            }
        }
    }

    void pushNextSampleIntoFifo (SampleType sample)
    {
        jassert (isPrepared());

        if (fifoIndex == size.get())
        {
            audioBufferFifo.push (bufferToFill);
            fifoIndex = 0;
        }
        auto writePointer = bufferToFill.getWritePointer (0);
        if (writePointer != nullptr)
        {
            writePointer[fifoIndex] = sample;
            fifoIndex++;
        }
    }

    void prepare (int bufferSize)
    {
        prepared = false;
        size = juce::jmax (1, bufferSize);
        bufferToFill.setSize (1, size.get());
        audioBufferFifo.prepare (size.get(), 1);
        fifoIndex = 0;
        prepared = true;
    }

    int getNumCompleteBuffersAvailable() const
    {
        if (! isPrepared())
        {
            return 0;
        }
        return audioBufferFifo.getNumAvailableForReading();
    }

    bool getAudioBuffer (BlockType& buf)
    {
        return audioBufferFifo.pull (buf);
    }

    bool isPrepared() const
    {
        return prepared.get();
    }
    int getSize() const
    {
        return size.get();
    }

private:
    Channel channelToUse;
    int fifoIndex = 0;
    Fifo<BlockType, 100> audioBufferFifo;
    BlockType bufferToFill;
    juce::Atomic<bool> prepared { false };
    juce::Atomic<int> size = 0;
};
