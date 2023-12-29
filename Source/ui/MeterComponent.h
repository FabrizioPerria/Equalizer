#pragma once

#include "data/Averager.h"
#include "data/DecayingValueHolder.h"
#include "utils/MeterConstants.h"

#include <JuceHeader.h>

struct MeterComponent : juce::Component
{
    MeterComponent (juce::String label);
    void resized() override;
    void paint (juce::Graphics&) override;
    void update (float peakDbLevel, float rmsDbLevel);

    juce::Rectangle<int> getGaugeBounds() const;

private:
    void buildForegroundImage();

    void paintRectangleForValue (juce::Graphics& g, float value, juce::Rectangle<float> rect, juce::Colour color);

    float peakDb { NEGATIVE_INFINITY };
    DecayingValueHolder peakDbDecay;

    Averager<float> averageDb { FRAMES_PER_SECOND * AVG_TIME_SECONDS, NEGATIVE_INFINITY };

    juce::String name;

    juce::Image bkgd;

    const float labelHeight { 14 };
    const float labelMargin { 5 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeterComponent)
};
