#pragma once

#include "ui/EqParamWidget.h"
#include <JuceHeader.h>

struct EqParamContainer : juce::Component
{
    EqParamContainer (juce::AudioProcessorValueTreeState& apvtsToUse);

    void paintOverChildren (juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& apvts;

    EqParamWidget eqParamWidget0 { apvts, 0, true };
    EqParamWidget eqParamWidget1 { apvts, 1, false };
    EqParamWidget eqParamWidget2 { apvts, 2, false };
    EqParamWidget eqParamWidget3 { apvts, 3, false };
    EqParamWidget eqParamWidget4 { apvts, 4, false };
    EqParamWidget eqParamWidget5 { apvts, 5, false };
    EqParamWidget eqParamWidget6 { apvts, 6, false };
    EqParamWidget eqParamWidget7 { apvts, 7, true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqParamContainer)
};
