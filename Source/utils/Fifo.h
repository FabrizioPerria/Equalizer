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

template <typename T>
struct IsReferenceCountedArray : std::false_type
{
};

template <typename T>
struct IsReferenceCountedArray<juce::ReferenceCountedArray<T>> : std::true_type
{
};

template <typename T>
struct IsVector : std::false_type
{
};

template <typename T>
struct IsVector<std::vector<T>> : std::true_type
{
};

template <typename T>
struct IsAudioBuffer : std::false_type
{
};

template <typename T>
struct IsAudioBuffer<juce::AudioBuffer<T>> : std::true_type
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

        for (auto& audioBuffer : buffer)
        {
            audioBuffer.setSize (numChannels, numSamples, false, false, true);
            audioBuffer.clear();
        }
    }

    //used when T is std::vector<float>
    void prepare (size_t numElements)
    {
        static_assert (std::is_same_v<T, std::vector<float>>, "T must be vector<float> when using prepare(numElements)");
        for (auto& item : buffer)
        {
            item.clear();
            item.resize (numElements, 0.0f);
        }
    }

    bool push (const T& t)
    {
        auto write = fifo.write (1);
        if (write.blockSize1 > 0)
        {
            size_t index = static_cast<size_t> (write.startIndex1);
            if constexpr (IsReferenceCountedObjectPtr<T>::value)
            {
                T old = buffer[index];
                buffer[index] = t;
                jassert (old.get() == nullptr || old->getReferenceCount() > 1);
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
            t = buffer[static_cast<size_t> (read.startIndex1)];
            return true;
        }
        return false;
    }

    bool exchange (T&& t)
    {
        auto read = fifo.read (1);

        if (read.blockSize1 > 0)
        {
            size_t index = static_cast<size_t> (read.startIndex1);
            if constexpr (IsReferenceCountedObjectPtr<T>::value)
            {
                std::swap (buffer[index], t);
                jassert (buffer[index].get() == nullptr);
            }
            else if constexpr (IsReferenceCountedArray<T>::value)
            {
                std::swap (buffer[index], t);
                jassert (buffer[index].isEmpty());
            }
            else if constexpr (IsVector<T>::value)
            {
                if (t.size() < buffer[index].size())
                {
                    t = buffer[index];
                }
                else
                {
                    std::swap (buffer[index], t);
                }
            }
            else if constexpr (IsAudioBuffer<T>::value)
            {
                if (t.getNumSamples() < buffer[index].getNumSamples())
                {
                    t = buffer[index];
                }
                else
                {
                    std::swap (buffer[index], t);
                }
            }
            else
            {
                std::swap (buffer[index], t);
                jassert (buffer[index] == nullptr);
            }
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
