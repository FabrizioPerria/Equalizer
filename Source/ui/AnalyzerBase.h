#pragma once
#include <JuceHeader.h>

#define RESPONSE_CURVE_MIN_DB -30.f
#define RESPONSE_CURVE_MAX_DB 30.f

struct AnalyzerBase : juce::Component
{
    juce::Rectangle<int> getBoundsForRendering() const
    {
        return getLocalBounds().reduced (0, getTextHeight() / 2 + 2);
    }

    juce::Rectangle<int> getBoundsForFFT()
    {
        auto boundsForRendering = getBoundsForRendering();
        auto w = static_cast<int> (getTextWidth() * 1.5f);
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
