#pragma once

#include "ui/EqControlLookAndFeel.h"
#include <JuceHeader.h>

struct VerticalSwitch : juce::Slider
{
    VerticalSwitch (juce::String newTitle);
    ~VerticalSwitch() override;

    void paint (juce::Graphics& g) override;

    juce::StringArray labels;

private:
    EqControlsLookAndFeel laf;

    juce::String title;
};
