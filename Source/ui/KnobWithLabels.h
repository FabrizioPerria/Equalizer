#pragma once

#include "ui/EqControlLookAndFeel.h"
#include <JuceHeader.h>

struct KnobWithLabels : juce::Slider
{
    KnobWithLabels (juce::String newTitle);
    ~KnobWithLabels() override;

    void paint (juce::Graphics& g) override;

    juce::StringArray labels;

private:
    const int textHeight { 12 };

    void updateTooltip();

    EqControlsLookAndFeel lnf;
    juce::String title;
};
