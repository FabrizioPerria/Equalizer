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
        auto bounds = slider.getLocalBounds().toFloat();

        auto relativeSliderPos = juce::jmap (sliderPos,
                                             static_cast<float> (x),
                                             static_cast<float> (x + width),
                                             bounds.getX(),
                                             bounds.getWidth());
        g.setColour (juce::Colour { 0x33, 0x33, 0x33 });
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

void CustomLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                              juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds();
    if (button.getToggleState())
        g.setColour (juce::Colours::green);
    else
        g.setColour (juce::Colours::black);

    g.fillRect (bounds);
    g.setColour (juce::Colours::white);
    g.drawRect (bounds, 1);
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
    : apvts (apvtsToUse), filterIndexInChain (filterIndex), isCutFilter (isCut)
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

    setupBypassButton (leftMidButton);
    setupBypassButton (rightSideButton);
    leftMidButton.setToggleState (true, juce::NotificationType::dontSendNotification);

    auto safePtr = juce::Component::SafePointer<EqParamWidget> (this);
    leftMidButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            comp->refreshSliders (Channel::LEFT);
        }
    };

    rightSideButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            comp->refreshSliders (Channel::RIGHT);
        }
    };

    auto eqModeParam = apvts.getParameter ("eq_mode");

    auto mode = static_cast<EqMode> (apvts.getRawParameterValue ("eq_mode")->load());
    refreshButtons (mode);

    auto dspModeCallback = [this] (float newDspMode)
    {
        auto dspMode = static_cast<EqMode> (newDspMode);
        refreshButtons (dspMode);
    };
    dspModeListener = std::make_unique<ParamListener<float>> (eqModeParam, dspModeCallback);
}

EqParamWidget::~EqParamWidget()
{
    setLookAndFeel (nullptr);
}

void EqParamWidget::resized()
{
    auto bounds = getLocalBounds();
    auto buttonsArea = bounds.removeFromBottom (20);
    auto leftButtonArea = buttonsArea.removeFromLeft (buttonsArea.getWidth() / 2);
    leftMidButton.setBounds (leftButtonArea.removeFromRight (leftButtonArea.getHeight()).reduced (2));
    auto rightButtonArea = buttonsArea;
    rightSideButton.setBounds (rightButtonArea.removeFromLeft (rightButtonArea.getHeight()).reduced (2));

    bounds.removeFromBottom (4);

    auto slidersArea = bounds;
    const auto sliderHeight = slidersArea.getHeight() / 3;
    auto frequencyArea = slidersArea.removeFromTop (sliderHeight);
    frequencySlider.setBounds (frequencyArea);
    auto qualityArea = slidersArea.removeFromTop (sliderHeight);
    qualitySlider.setBounds (qualityArea);
    auto slopeOrGainArea = slidersArea;
    slopeOrGainSlider->setBounds (slopeOrGainArea);
}

void EqParamWidget::refreshButtons (EqMode dspMode)
{
    if (dspMode == EqMode::STEREO)
    {
        leftMidButton.setVisible (false);
        rightSideButton.setVisible (false);
        leftMidButton.setToggleState (true, juce::NotificationType::sendNotification);
    }
    else
    {
        leftMidButton.setVisible (true);
        leftMidButton.setButtonText (dspMode == EqMode::DUAL_MONO ? "L" : "M");
        rightSideButton.setVisible (true);
        rightSideButton.setButtonText (dspMode == EqMode::DUAL_MONO ? "R" : "S");
    }
}

void EqParamWidget::refreshSliders (Channel channel)
{
    frequencyAttachment.reset();
    qualityAttachment.reset();
    slopeOrGainAttachment.reset();

    auto frequencyName = FilterInfo::getParameterName (filterIndexInChain, channel, FilterInfo::FilterParam::FREQUENCY);
    frequencyAttachment = std::make_unique<SliderAttachment> (apvts, //
                                                              frequencyName,
                                                              frequencySlider);

    auto qualityName = FilterInfo::getParameterName (filterIndexInChain, channel, FilterInfo::FilterParam::Q);
    qualityAttachment = std::make_unique<SliderAttachment> (apvts, //
                                                            qualityName,
                                                            qualitySlider);

    auto slopeOrGainName = FilterInfo::getParameterName (filterIndexInChain,
                                                         channel,
                                                         isCutFilter ? FilterInfo::FilterParam::SLOPE : FilterInfo::FilterParam::GAIN);
    slopeOrGainAttachment = std::make_unique<SliderAttachment> (apvts, slopeOrGainName, *slopeOrGainSlider);
}

/* void EqParamWidget::buildGridImage() */
/* { */
/*     auto globalDisplayScale = juce::Desktop::getInstance().getGlobalScaleFactor(); */
/*     auto width = static_cast<int> (getWidth() * globalDisplayScale); */
/*     auto height = static_cast<int> (getHeight() * globalDisplayScale); */
/*     widgetGridImage = juce::Image (juce::Image::PixelFormat::RGB, width, height, true); */
/*     auto g = juce::Graphics (widgetGridImage); */
/**/
/*     g.addTransform (juce::AffineTransform::scale (globalDisplayScale)); */
/**/
/*     g.setColour (juce::Colours::aquamarine); */
/**/
/*     auto bounds = getLocalBounds(); */
/*     bounds.removeFromBottom (24); */
/**/
/*     auto slidersArea = bounds; */
/*     const auto sliderHeight = slidersArea.getHeight() / 3; */
/**/
/*     auto frequencyArea = slidersArea.removeFromTop (sliderHeight); */
/*     g.drawLine (frequencyArea.getX(), frequencyArea.getBottom(), frequencyArea.getRight(), frequencyArea.getBottom(), 1); */
/*     g.drawLine (frequencyArea.getRight(), frequencyArea.getY(), frequencyArea.getRight(), frequencyArea.getBottom(), 1); */
/*     auto qualityArea = slidersArea.removeFromTop (sliderHeight); */
/*     g.drawLine (qualityArea.getX(), qualityArea.getBottom(), qualityArea.getRight(), qualityArea.getBottom(), 1); */
/*     g.drawLine (qualityArea.getRight(), qualityArea.getY(), qualityArea.getRight(), qualityArea.getBottom(), 1); */
/*     auto slopeOrGainArea = slidersArea; */
/*     g.drawLine (slopeOrGainArea.getX(), slopeOrGainArea.getBottom(), slopeOrGainArea.getRight(), slopeOrGainArea.getBottom(), 1); */
/*     g.drawLine (slopeOrGainArea.getRight(), slopeOrGainArea.getY(), slopeOrGainArea.getRight(), slopeOrGainArea.getBottom(), 1); */
/* } */

void EqParamWidget::setupBypassButton (juce::TextButton& button)
{
    addChildComponent (button);
    button.setRadioGroupId (1);
    button.setClickingTogglesState (true);
}
