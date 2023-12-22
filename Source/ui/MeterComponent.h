#pragma once

#include "data/DecayingValueHolder.h"
#include "utils/MeterConstants.h"

#include <JuceHeader.h>

struct MeterComponent : juce::Component
{
    void paint (juce::Graphics&) override;
    void update (float dbLevel);

private:
    float peakDb { NEGATIVE_INFINITY };
    DecayingValueHolder peakDbDecay;
};
