#pragma once

#include "ui/EqParamWidget.h"
#include "ui/NodeController.h"
#include <JuceHeader.h>

struct EqParamContainer : juce::Component, NodeController::NodeListener
{
    EqParamContainer (juce::AudioProcessorValueTreeState& apvtsToUse);

    void paintOverChildren (juce::Graphics& g) override;
    void resized() override;

    static const int buttonArea = EqParamWidget::buttonSideLength + EqParamWidget::buttonMargin;
    static const int sliderArea = 3 * EqParamWidget::sliderHeight;
    void bandMouseOver (ChainPositions, Channel) override;
    void bandSelected (ChainPositions, Channel) override;
    void clearSelection() override;

private:
    juce::AudioProcessorValueTreeState& apvts;

    std::array<EqParamWidget, 8> widgets { { { apvts, static_cast<int> (ChainPositions::LOWCUT), true },    //
                                             { apvts, static_cast<int> (ChainPositions::LOWSHELF), false }, //
                                             { apvts, static_cast<int> (ChainPositions::PEAK1), false },    //
                                             { apvts, static_cast<int> (ChainPositions::PEAK2), false },
                                             { apvts, static_cast<int> (ChainPositions::PEAK3), false }, //
                                             { apvts, static_cast<int> (ChainPositions::PEAK4), false },
                                             { apvts, static_cast<int> (ChainPositions::HIGHSHELF), false },
                                             { apvts, static_cast<int> (ChainPositions::HIGHCUT), true } } };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqParamContainer)
};
