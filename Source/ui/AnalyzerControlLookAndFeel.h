#pragma once

#include <JuceHeader.h>

struct AnalyzerControlsLookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           const float rotaryStartAngle,
                           const float rotaryEndAngle,
                           juce::Slider& slider) override;

    void drawLinearSlider (juce::Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           const juce::Slider::SliderStyle style,
                           juce::Slider& slider) override;

private:
    juce::Colour darkdark = juce::Colour::fromString (("ff0d0e16"));
    juce::Colour dark = juce::Colour::fromString (("ff14141f"));
    juce::Colour light = juce::Colour::fromString (("88367e3b3"));
};
