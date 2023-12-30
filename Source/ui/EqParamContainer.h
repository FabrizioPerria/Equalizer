#pragma once

#include "PluginProcessor.h"
#include "ui/EqParamWidget.h"
#include <JuceHeader.h>

struct EqParamContainer : juce::Component
{
    EqParamContainer (juce::AudioProcessorValueTreeState& apvtsToUse);

    void paintOverChildren (juce::Graphics& g) override;
    void resized() override;

    static const int buttonArea = EqParamWidget::buttonSideLength + EqParamWidget::buttonMargin;
    static const int sliderArea = 3 * EqParamWidget::sliderHeight;

private:
    juce::AudioProcessorValueTreeState& apvts;

    EqParamWidget lowcutWidget { apvts, static_cast<int> (ChainPositions::LOWCUT), true };
    EqParamWidget lowShelfWidget { apvts, static_cast<int> (ChainPositions::LOWSHELF), false };
    EqParamWidget peakWidget1 { apvts, static_cast<int> (ChainPositions::PEAK1), false };
    EqParamWidget peakWidget2 { apvts, static_cast<int> (ChainPositions::PEAK2), false };
    EqParamWidget peakWidget3 { apvts, static_cast<int> (ChainPositions::PEAK3), false };
    EqParamWidget peakWidget4 { apvts, static_cast<int> (ChainPositions::PEAK4), false };
    EqParamWidget highShelfWidget { apvts, static_cast<int> (ChainPositions::HIGHSHELF), false };
    EqParamWidget highCutWidget { apvts, static_cast<int> (ChainPositions::HIGHCUT), true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqParamContainer)
};
