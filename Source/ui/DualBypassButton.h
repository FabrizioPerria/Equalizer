#pragma once

#include "PluginProcessor.h"
#include "data/ParamListener.h"
#include "ui/BypassButton.h"
#include <JuceHeader.h>

struct DualBypassButton : juce::Component
{
    DualBypassButton (ChainPositions pos, juce::AudioProcessorValueTreeState& apvtsToUse);

    void paintOverChildren (juce::Graphics& g) override;

    void resized() override;
    void refreshButtons (EqMode mode);

    bool isShowingAsOn (Channel channel);

private:
    juce::AudioProcessorValueTreeState& apvts;

    bool isPaired { false };

    BypassButton leftButton;
    BypassButton rightButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> leftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> rightAttachment;

    ChainPositions chainPosition;

    std::unique_ptr<ParamListener<float>> eqModeListener;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DualBypassButton)
};
