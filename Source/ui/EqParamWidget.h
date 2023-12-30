#pragma once

#include "data/ParamListener.h"
#include "utils/EqParam.h"
#include <JuceHeader.h>

struct TextOnlyHorizontalSlider : juce::Slider
{
    TextOnlyHorizontalSlider() : juce::Slider (juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        /*
         setting this to false prevents the slider from snapping its value to wherever you click inside the slider bounds.
         */
        setSliderSnapsToMousePosition (false);
    }
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
                           Slider& slider) override
    {
        if (TextOnlyHorizontalSlider* textSlider = dynamic_cast<TextOnlyHorizontalSlider*> (&slider))
        {
            auto text = textSlider->getDisplayString();
            auto bounds = slider.getLocalBounds().toFloat().reduced (2.0f);

            auto relativeSliderPos = juce::jmap (sliderPos,
                                                 static_cast<float> (x),
                                                 static_cast<float> (x + width),
                                                 bounds.getX(),
                                                 bounds.getWidth());
            g.setColour (juce::Colours::darkgrey);
            g.fillRect (bounds.withWidth (relativeSliderPos));

            g.setColour (juce::Colours::white);
            g.setFont (12.0f);
            g.drawFittedText (text, bounds.toNearestInt(), juce::Justification::centred, true);
        }
        else
        {
            juce::LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }
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

class EqParamWidget : public juce::Component
{
    EqParamWidget (juce::AudioProcessorValueTreeState& apvtsToUse);

    void paint (juce::Graphics& g) override;
    void resized() override
    {
        // resize bounds of all widgets
        buildGridImage();
    }

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

    ParamListener<float> dspModeListener;
    ParamListener<float> leftMidBypassListener;
    ParamListener<float> rightSideBypassListener;

    juce::TextButton leftMidBypass;
    juce::TextButton rightSideBypass;

    CustomLookAndFeel customLookAndFeel;

    juce::Image widgetGridImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqParamWidget)
};
