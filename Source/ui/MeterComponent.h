#pragma once

#include <JuceHeader.h>

#define NEGATIVE_INFINITY -66.0f
#define MAX_DECIBELS 12.0f

struct MeterComponent : public juce::Component
{
    void paint (juce::Graphics&) override;
    void update (float dbLevel);

private:
    float peakDb { NEGATIVE_INFINITY };
};
