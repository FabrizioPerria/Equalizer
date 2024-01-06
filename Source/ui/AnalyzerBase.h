#pragma once
#include <JuceHeader.h>

struct AnalyzerBase : juce::Component
{
    juce::Rectangle<int> getBoundsForRendering() const
    {
        return getLocalBounds().reduced (0, getTextHeight() / 2 + 2);
    }

    juce::Rectangle<int> getBoundsForFFT()
    {
        auto boundsForRendering = getBoundsForRendering();
        auto w = getTextWidth() * 1.5f;
        return boundsForRendering.withTrimmedLeft (w).withTrimmedRight (w);
    }

    inline int getTextHeight() const
    {
        return 16;
    }
    inline int getTextWidth() const
    {
        return 16;
    }
    void resized() override
    {
        fftBoundingBox = getBoundsForFFT();
    }

protected:
    juce::Rectangle<int> fftBoundingBox;
};
