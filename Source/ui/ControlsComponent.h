#pragma once

#include "ui/AnalyzerControls.h"
#include <JuceHeader.h>

struct ControlsComponent : juce::Component
{
    ControlsComponent (juce::AudioProcessorValueTreeState& apv) : apvts { apv }
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

        bounds.removeFromLeft (10);

        auto outputGainKnobBounds = bounds.removeFromRight (gainSize);
        outputGainKnob.setBounds (outputGainKnobBounds);
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (juce::Colours::aquamarine);
        g.drawHorizontalLine (0, 0, getWidth());

        g.drawVerticalLine (gainSize + 10 + eqModeSize + 5, 0, getHeight());
        g.drawVerticalLine (gainSize + 10 + eqModeSize + 10 + analyzerControlsSize + 5, 0, getHeight());

        g.drawVerticalLine (gainSize + 5, 0, getHeight());
        g.drawVerticalLine (getWidth() - gainSize - 5, 0, getHeight());
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

    float gainSize = 80.0f;
    float eqModeSize = 80.0f;
    float analyzerControlsSize = 400.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlsComponent)
};
