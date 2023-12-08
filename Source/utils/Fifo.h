#pragma once

#include <JuceHeader.h>
#include <cstddef>

template <typename T>
struct IsReferenceCountedObjectPtr : std::false_type
{
};

template <typename T>
struct IsReferenceCountedObjectPtr<juce::ReferenceCountedObjectPtr<T>> : std::true_type
{
};

template <typename T, size_t Size>
struct Fifo
{
    size_t getSize() const noexcept
    {
        return Size;
    }

    //used when T is AudioBuffer<float>
    void prepare (int numSamples, int numChannels)
    {
        static_assert (std::is_same_v<T, juce::AudioBuffer<float>>,
                       "T must be AudioBuffer<float> when using prepare(numSamples, numChannels)");

        buffer.setSize (numChannels, numSamples, false, true, true);
        buffer.clear();
    }

    //used when T is std::vector<float>
    void prepare (size_t numElements)
    {
        static_assert (std::is_same_v<T, std::vector<float>>, "T must be vector<float> when using prepare(numElements)");

        buffer.clear();
        buffer.resize (numElements, 0);
    }

    bool push (const T& t)
    {
        auto write = fifo.write (1);
        if (write.blockSize1 > 0)
        {
            size_t index = static_cast<size_t>(write.startIndex1);
            if constexpr( IsReferenceCountedObjectPtr<T>::value)
            {
                T old = buffer[index];
                buffer[index] = t;
                jassert(old.get() == nullptr || old->getReferenceCount() > 1);
            }
            else
            {
                buffer[index] = t;
            }
            return true;
        }
        return false;
    }

    bool pull (T& t)
    {
        auto read = fifo.read (1);
        if (read.blockSize1 > 0)
        {
            t = buffer[read.startIndex1];
            return true;
        }
        return false;
    }

    int getNumAvailableForReading() const
    {
        return fifo.getNumReady();
    }

    int getAvailableSpace() const
    {
        return fifo.getFreeSpace();
    }

private:
    juce::AbstractFifo fifo { Size };
    std::array<T, Size> buffer;
};
