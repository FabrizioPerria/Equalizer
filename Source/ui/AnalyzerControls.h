#pragma once

#include <JuceHeader.h>

struct AnalyzerControls : juce::Component
{
    AnalyzerControls (juce::AudioProcessorValueTreeState& apv);
    ~AnalyzerControls() override;

    void resized() override;

private:
    void toggleEnable();

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    juce::TextButton enableButton { "On" };
    std::unique_ptr<ButtonAttachment> enableButtonAttachment;
    juce::Slider inputSlider;
    std::unique_ptr<SliderAttachment> inputSliderAttachment;
    juce::Slider pointsSlider;
    std::unique_ptr<SliderAttachment> pointsSliderAttachment;
    juce::Slider decaySlider;
    std::unique_ptr<SliderAttachment> decaySliderAttachment;
};
