#pragma once

#include "data/Averager.h"
#include "data/DecayingValueHolder.h"
#include "utils/MeterConstants.h"

#include <JuceHeader.h>

struct MeterComponent : juce::Component
{
    void paint (juce::Graphics&) override;
    void update (float dbLevel);

private:
    void paintRectangleForValue (juce::Graphics& g, float value, juce::Rectangle<float> rect, juce::Colour color);

    float peakDb { NEGATIVE_INFINITY };
    DecayingValueHolder peakDbDecay;

    Averager<float> averageDb { FRAMES_PER_SECOND * AVG_TIME_SECONDS, NEGATIVE_INFINITY };
};
