#pragma once

#include "ui/MeterComponent.h"
#include <JuceHeader.h>

struct DecayingValueHolder : juce::Timer
{
    DecayingValueHolder();

    void updateHeldValue (float input);

    float getCurrentValue() const;
    bool isOverThreshold() const;

    void setHoldTime (int ms);
    void setDecayRate (float dbPerSec);

    void timerCallback() override;

private:
    float currentValue { NEGATIVE_INFINITY };
    juce::int64 peakTime = getNow();
    float threshold = 0.f;
    juce::int64 holdTime = 2000; //2 seconds
    float decayRatePerFrame { 0 };
    float decayRateMultiplier { 1 };
    float decayAcceleration { 1.01f };

    static juce::int64 getNow();
    void resetDecayRateMultiplier();
};
