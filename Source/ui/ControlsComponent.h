#pragma once

#include "ui/AnalyzerControls.h"
#include "ui/KnobWithLabels.h"
#include "ui/NodeController.h"
#include "ui/VerticalSwitch.h"
#include <JuceHeader.h>

struct ControlsComponent : juce::Component
{
    ControlsComponent (juce::AudioProcessorValueTreeState& apv, NodeController& controller) : apvts { apv }, nodeController { controller }
    {
        addAndMakeVisible (analyzerControls);
        addAndMakeVisible (inputGainKnob);
        inputGainKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts,
                                                                                                          "input_gain",
                                                                                                          inputGainKnob);

        addAndMakeVisible (outputGainKnob);
        outputGainKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts,
                                                                                                           "output_gain",
                                                                                                           outputGainKnob);

        addAndMakeVisible (eqMode);
        eqModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "eq_mode", eqMode);

        addAndMakeVisible (analyzerControls);

        addAndMakeVisible (resetButton);
        resetButton.onClick = [&] { nodeController.resetAllParameters(); };

        inputGainKnob.labels.add ("-18");
        inputGainKnob.labels.add ("+18");

        outputGainKnob.labels.add ("-18");
        outputGainKnob.labels.add ("+18");

        eqMode.labels.add ("Stereo");
        eqMode.labels.add ("Dual Mono");
        eqMode.labels.add ("Mid/Side");
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        bounds.removeFromTop (10);

        auto inputGainKnobBounds = bounds.removeFromLeft (gainSize);
        inputGainKnob.setBounds (inputGainKnobBounds);

        bounds.removeFromLeft (10);

        auto eqModeBounds = bounds.removeFromLeft (eqModeSize);
        eqMode.setBounds (eqModeBounds);

        bounds.removeFromLeft (10);

        auto analyzerControlsBounds = bounds.removeFromLeft (analyzerControlsSize);
        analyzerControls.setBounds (analyzerControlsBounds);

        bounds.removeFromLeft (5);

        auto outputGainKnobBounds = bounds.removeFromRight (gainSize);
        outputGainKnob.setBounds (outputGainKnobBounds);

        int centerX = bounds.getCentreX();
        int centerY = bounds.getCentreY();

        resetButton.setSize (resetButtonSize, resetButtonSize / 2);

        resetButton.setCentrePosition (centerX, centerY);
    }

    void paint (juce::Graphics& g) override
    {
        auto width = static_cast<float> (getWidth());
        auto height = static_cast<float> (getHeight());

        g.setColour (juce::Colours::aquamarine);
        g.drawHorizontalLine (0, 0, width);

        g.drawVerticalLine (gainSize + 10 + eqModeSize + 5, 0, height);
        g.drawVerticalLine (gainSize + 10 + eqModeSize + 10 + analyzerControlsSize + 5, 0, height);

        g.drawVerticalLine (gainSize + 5, 0, height);
        g.drawVerticalLine (getWidth() - gainSize - 5, 0, height);
    }

private:
    juce::AudioProcessorValueTreeState& apvts;

    AnalyzerControls analyzerControls { apvts };
    KnobWithLabels inputGainKnob { "Input Gain" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainKnobAttachment;
    KnobWithLabels outputGainKnob { "Output Gain" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainKnobAttachment;

    VerticalSwitch eqMode { "EQ Mode" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqModeAttachment;

    juce::TextButton resetButton { "Reset Eq" };

    int gainSize = 80;
    int eqModeSize = 80;
    int resetButtonSize = 80;
    int analyzerControlsSize = 400;

    NodeController& nodeController;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlsComponent)
};
