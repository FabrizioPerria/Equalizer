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
};
