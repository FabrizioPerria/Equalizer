#pragma once

#include "ui/DbScaleComponent.h"
#include "ui/MeterComponent.h"

struct StereoMeterComponent : juce::Component
{
    StereoMeterComponent();

    void paint (juce::Graphics&) override;
    void resized() override;
    void update (float leftDbLevel, float rightDbLevel);

private:
    MeterComponent leftMeter;
    MeterComponent rightMeter;

    DbScaleComponent dbScale;
};
