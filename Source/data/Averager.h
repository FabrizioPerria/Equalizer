#pragma once

#include <JuceHeader.h>
#include <vector>

template <typename ValueType>
struct Averager
{
    Averager (size_t numElements, ValueType initialValue)
    {
        resize (numElements, initialValue);
    }

    void resize (size_t numElements, ValueType initialValue)
    {
        elements.resize (numElements);
        clear (initialValue);
    }

    void clear (ValueType initialValue)
    {
        auto size = getSize();
        for (auto& element : elements)
        {
            element = initialValue;
        }
        writeIndex = 0;
        sum = initialValue * size;
        avg = initialValue;
    }

    size_t getSize() const
    {
        return elements.size();
    }

    void add (ValueType t)
    {
        auto currentSize = getSize();
        auto currentwriteIndex = writeIndex.load();
        auto currentSum = sum.load();
        currentSum -= elements[currentwriteIndex];
        currentSum += t;
        auto currentAverage = currentSum / currentSize;
        elements[currentwriteIndex] = t;

        currentwriteIndex = (currentwriteIndex + 1) % currentSize;

        sum = currentSum;
        writeIndex = currentwriteIndex;
        avg = currentAverage;
    }

    float getAvg() const
    {
        return avg.load();
    }

private:
    std::vector<ValueType> elements;
    std::atomic<float> avg { static_cast<float> (ValueType()) };
    std::atomic<size_t> writeIndex = 0;
    std::atomic<ValueType> sum { 0 };
};
