#pragma once

#include "data/ParamListener.h"
#include "ui/EqControlLookAndFeel.h"
#include "utils/EqParam.h"
#include <JuceHeader.h>

struct TextOnlyHorizontalSlider : juce::Slider
{
    TextOnlyHorizontalSlider();
    virtual ~TextOnlyHorizontalSlider() = default;
    virtual juce::String getDisplayString() = 0;
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

    void resized() override;

    static const int buttonSideLength { 20 };
    static const int buttonMargin { 4 };
    static const int sliderHeight { 20 };

private:
    void refreshButtons (EqMode dspMode);
    void refreshSliders (Channel channel);
    void setupBypassButton (juce::TextButton& button);

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

    juce::TextButton leftMidButton;
    juce::TextButton rightSideButton;

    EqControlsLookAndFeel customLookAndFeel;

    juce::Image widgetGridImage;

    int filterIndexInChain;
    bool isCutFilter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqParamWidget)
};
