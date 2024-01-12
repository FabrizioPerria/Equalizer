#pragma once

#include "ui/KnobWithLabels.h"
#include "ui/VerticalSwitch.h"
#include <JuceHeader.h>

struct AnalyzerControls : juce::Component
{
    AnalyzerControls (juce::AudioProcessorValueTreeState& apv);
    ~AnalyzerControls() override;

    void resized() override;

private:
    void toggleEnable();

    EqControlsLookAndFeel laf;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    juce::TextButton enableButton { "On" };
    std::unique_ptr<ButtonAttachment> enableButtonAttachment;
    VerticalSwitch inputSlider { "Input" };
    std::unique_ptr<SliderAttachment> inputSliderAttachment;
    VerticalSwitch pointsSlider { "Points" };
    std::unique_ptr<SliderAttachment> pointsSliderAttachment;
    KnobWithLabels decaySlider { "Decay Rate" };
    std::unique_ptr<SliderAttachment> decaySliderAttachment;
};
