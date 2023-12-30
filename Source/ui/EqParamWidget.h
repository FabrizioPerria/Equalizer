#pragma once

#include "data/ParamListener.h"
#include "utils/EqParam.h"
#include <JuceHeader.h>

struct TextOnlyHorizontalSlider : juce::Slider
{
    TextOnlyHorizontalSlider();
    virtual ~TextOnlyHorizontalSlider() = default;
    virtual juce::String getDisplayString() = 0;
};

struct CustomLookAndFeel : juce::LookAndFeel_V4
{
    void drawLinearSlider (Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           Slider::SliderStyle style,
                           Slider& slider) override;
};

struct HertzSlider : TextOnlyHorizontalSlider
{
    juce::String getDisplayString() override;
};

struct QualitySlider : TextOnlyHorizontalSlider
{
    juce::String getDisplayString() override;
};

struct SlopeSlider : TextOnlyHorizontalSlider
{
    juce::String getDisplayString() override;
};

struct GainSlider : TextOnlyHorizontalSlider
{
    juce::String getDisplayString() override;
};

struct EqParamWidget : juce::Component
{
    EqParamWidget (juce::AudioProcessorValueTreeState& apvtsToUse, int filterIndex, bool isCut);
    ~EqParamWidget() override;

    void paint (juce::Graphics& g) override;

    void resized() override;

    void refreshButtons (EqMode dspMode);
    void refreshSliders (Channel channel);

private:
    void buildGridImage();

    juce::AudioProcessorValueTreeState& apvts;

    HertzSlider frequencySlider;
    QualitySlider qualitySlider;
    std::unique_ptr<juce::Slider> slopeOrGainSlider;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> frequencyAttachment;
    std::unique_ptr<SliderAttachment> qualityAttachment;
    std::unique_ptr<SliderAttachment> slopeOrGainAttachment;

    std::unique_ptr<ParamListener<float>> dspModeListener;
    std::unique_ptr<ParamListener<float>> leftMidBypassListener;
    std::unique_ptr<ParamListener<float>> rightSideBypassListener;

    juce::TextButton leftMidBypass;
    juce::TextButton rightSideBypass;

    CustomLookAndFeel customLookAndFeel;

    juce::Image widgetGridImage;

    int filterIndex;
    bool isCut;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqParamWidget)
};
