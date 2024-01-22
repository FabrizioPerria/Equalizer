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

    currentChannelSelected = Channel::LEFT;
    refreshSliders (currentChannelSelected);

    setupBypassButton (leftMidButton);
    setupBypassButton (rightSideButton);
    leftMidButton.setToggleState (true, juce::NotificationType::dontSendNotification);

    auto safePtr = juce::Component::SafePointer<EqParamWidget> (this);
    leftMidButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            comp->currentChannelSelected = Channel::LEFT;
            comp->refreshSliders (Channel::LEFT);
        }
    };

    rightSideButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            comp->currentChannelSelected = Channel::RIGHT;
            comp->refreshSliders (Channel::RIGHT);
        }
    };

    auto mode = static_cast<EqMode> (apvts.getRawParameterValue ("eq_mode")->load());
    refreshButtons (mode);

    auto eqModeParam = apvts.getParameter ("eq_mode");
    auto dspModeCallback = [this] (float newDspMode)
    {
        auto dspMode = static_cast<EqMode> (newDspMode);
        refreshButtons (dspMode);
    };
    dspModeListener = std::make_unique<ParamListener<float>> (eqModeParam, dspModeCallback);

    auto widgetSafePtr = juce::Component::SafePointer<EqParamWidget> (this);
    auto leftBypassName = FilterInfo::getParameterName (filterIndexInChain, Channel::LEFT, FilterInfo::FilterParam::BYPASS);
    auto leftBypassParam = apvts.getParameter (leftBypassName);
    leftBypassListener = std::make_unique<ParamListener<float>> (leftBypassParam,
                                                                 [widgetSafePtr] (bool v)
                                                                 {
                                                                     if (auto* comp = widgetSafePtr.getComponent())
                                                                     {
                                                                         comp->refreshSliders (Channel::LEFT);
                                                                     }
                                                                 });

    auto rightBypassName = FilterInfo::getParameterName (filterIndexInChain, Channel::RIGHT, FilterInfo::FilterParam::BYPASS);
    auto rightBypassParam = apvts.getParameter (rightBypassName);
    rightBypassListener = std::make_unique<ParamListener<float>> (rightBypassParam,
                                                                  [widgetSafePtr] (bool v)
                                                                  {
                                                                      if (auto* comp = widgetSafePtr.getComponent())
                                                                      {
                                                                          comp->refreshSliders (Channel::RIGHT);
                                                                      }
                                                                  });
}

EqParamWidget::~EqParamWidget()
{
    setLookAndFeel (nullptr);
}

void EqParamWidget::resized()
{
    auto bounds = getLocalBounds();
    auto buttonsArea = bounds.removeFromBottom (buttonSideLength);

    auto leftButtonArea = buttonsArea.removeFromLeft (buttonsArea.getWidth() / 2);
    leftMidButton.setBounds (leftButtonArea.removeFromRight (buttonSideLength).reduced (1));
    auto rightButtonArea = buttonsArea;
    rightSideButton.setBounds (rightButtonArea.removeFromLeft (buttonSideLength).reduced (1));

    bounds.removeFromBottom (buttonMargin);

    auto slidersArea = bounds;
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

    if (channel == currentChannelSelected)
    {
        auto bypassName = FilterInfo::getParameterName (filterIndexInChain, channel, FilterInfo::FilterParam::BYPASS);
        auto bypassValue = apvts.getRawParameterValue (bypassName)->load() > 0.5f;
        setEnabled (! bypassValue);
    }
}

void EqParamWidget::setupBypassButton (juce::TextButton& button)
{
    addChildComponent (button);
    button.setRadioGroupId (1);
    button.setClickingTogglesState (true);
}

void EqParamWidget::setEnabled (bool shouldBeEnabled)
{
    frequencySlider.setEnabled (shouldBeEnabled);
    qualitySlider.setEnabled (shouldBeEnabled);
    slopeOrGainSlider->setEnabled (shouldBeEnabled);
}

void EqParamWidget::clearBand()
{
    selected = false;
    refreshSliders (currentChannelSelected);
}

void EqParamWidget::selectBand (Channel ch)
{
    selected = true;
    if (currentChannelSelected == ch)
    {
        refreshSliders (ch);
    }
    else
    {
        currentChannelSelected = ch;
        auto& toggleButton = ch == Channel::LEFT ? leftMidButton : rightSideButton;
        toggleButton.setToggleState (true, juce::NotificationType::sendNotification);
    }
}
