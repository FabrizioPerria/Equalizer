#pragma once

#include "data/MeterValues.h"
#include "ui/DbScaleComponent.h"
#include "ui/MeterComponent.h"

struct StereoMeterComponent : juce::Component
{
    StereoMeterComponent (juce::String label);

    void paint (juce::Graphics&) override;
    void resized() override;
    void update (MeterValues meterValues);

private:
    MeterComponent leftMeter { "L" };
    MeterComponent rightMeter { "R" };

    DbScaleComponent dbScale;

    juce::String name;

    const int labelHeight { 14 };
    const int componentsMargin { 10 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoMeterComponent)
};
