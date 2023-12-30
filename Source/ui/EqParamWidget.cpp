#include "ui/EqParamWidget.h"
#include "utils/EqParam.h"
#include "utils/FilterParam.h"
#include <memory>

TextOnlyHorizontalSlider::TextOnlyHorizontalSlider()
    : juce::Slider (juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::NoTextBox)
{
    /*
     setting this to false prevents the slider from snapping its value to wherever you click inside the slider bounds.
     */
    setSliderSnapsToMousePosition (false);
}

void CustomLookAndFeel::drawLinearSlider (Graphics& g,
                                          int x,
                                          int y,
                                          int width,
                                          int height,
                                          float sliderPos,
                                          float minSliderPos,
                                          float maxSliderPos,
                                          Slider::SliderStyle style,
                                          Slider& slider)
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

juce::String HertzSlider::getDisplayString()
{
    auto frequency = getValue();
    if (frequency >= 1000.0f)
    {
        return juce::String (frequency / 1000.0f, 2) + " kHz";
    }
    return juce::String (frequency, 0) + " Hz";
}

juce::String QualitySlider::getDisplayString()
{
    auto q = getValue();
    return juce::String (q, 2);
}

juce::String SlopeSlider::getDisplayString()
{
    auto slopeIndex = static_cast<int> (getValue());
    auto slope = 6 + slopeIndex * 6;
    return juce::String (slope) + " dB/Oct";
}

juce::String GainSlider::getDisplayString()
{
    auto gain = getValue();
    return juce::String (gain, 1) + " dB";
}

EqParamWidget::EqParamWidget (juce::AudioProcessorValueTreeState& apvtsToUse, int filterIndex, bool isCut)
    : apvts (apvtsToUse), filterIndex (filterIndex), isCut (isCut)
{
    setLookAndFeel (&customLookAndFeel);

    addAndMakeVisible (frequencySlider);
    addAndMakeVisible (qualitySlider);
    if (isCut)
    {
        slopeOrGainSlider.reset (new SlopeSlider());
    }
    else
    {
        slopeOrGainSlider.reset (new GainSlider());
    }
    addAndMakeVisible (*slopeOrGainSlider);

    refreshSliders (Channel::LEFT);

    //TODO: DRY
    addChildComponent (leftMidBypass);
    leftMidBypass.setRadioGroupId (1);
    leftMidBypass.setClickingTogglesState (true);
    addChildComponent (rightSideBypass);
    rightSideBypass.setRadioGroupId (1);
    leftMidBypass.setClickingTogglesState (true);

    auto safePtr = juce::Component::SafePointer<EqParamWidget> (this);
    leftMidBypass.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            comp->refreshSliders (Channel::LEFT);
        }
    };

    rightSideBypass.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            comp->refreshSliders (Channel::RIGHT);
        }
    };

    auto eqModeParam = apvts.getParameter ("eq_mode");
    auto mode = static_cast<EqMode> (eqModeParam->getValue());
    refreshButtons (mode);

    dspModeListener = std::make_unique<ParamListener<float>> (eqModeParam,
                                                              [this] (float newValue) { refreshButtons (static_cast<EqMode> (newValue)); });
}

EqParamWidget::~EqParamWidget()
{
    setLookAndFeel (nullptr);
}

void EqParamWidget::paint (juce::Graphics& g)
{
    /* g.drawImage (widgetGridImage, getLocalBounds().toFloat()); */
}

void EqParamWidget::resized()
{
    // resize bounds of all widgets
    // then
    /* buildGridImage(); */
}

void EqParamWidget::refreshButtons (EqMode dspMode)
{
    if (dspMode == EqMode::STEREO)
    {
        leftMidBypass.setVisible (false);
        rightSideBypass.setVisible (false);
        leftMidBypass.setToggleState (true, juce::NotificationType::sendNotification);
    }
    else
    {
        leftMidBypass.setVisible (true);
        leftMidBypass.setButtonText (dspMode == EqMode::DUAL_MONO ? "L" : "M");
        rightSideBypass.setVisible (true);
        rightSideBypass.setButtonText (dspMode == EqMode::DUAL_MONO ? "R" : "S");
    }
}

void EqParamWidget::refreshSliders (Channel channel)
{
    frequencyAttachment.reset();
    qualityAttachment.reset();
    slopeOrGainAttachment.reset();

    auto frequencyName = FilterInfo::getParameterName (filterIndex, channel, FilterInfo::FilterParam::FREQUENCY);
    frequencyAttachment = std::make_unique<SliderAttachment> (apvts, //
                                                              frequencyName,
                                                              frequencySlider);

    auto qualityName = FilterInfo::getParameterName (filterIndex, channel, FilterInfo::FilterParam::Q);
    qualityAttachment = std::make_unique<SliderAttachment> (apvts, //
                                                            qualityName,
                                                            qualitySlider);

    auto slopeOrGainName = FilterInfo::getParameterName (filterIndex,
                                                         channel,
                                                         isCut ? FilterInfo::FilterParam::SLOPE : FilterInfo::FilterParam::GAIN);
    slopeOrGainAttachment = std::make_unique<SliderAttachment> (apvts, slopeOrGainName, *slopeOrGainSlider);
}

void EqParamWidget::buildGridImage()
{
}
