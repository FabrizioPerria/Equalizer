#include "ui/AnalyzerControls.h"
#include "utils/AnalyzerProperties.h"

AnalyzerControls::AnalyzerControls (juce::AudioProcessorValueTreeState& apv)
{
    using ParamNames = AnalyzerProperties::ParamNames;

    auto params = AnalyzerProperties::GetAnalyzerParams();
    enableButtonAttachment = std::make_unique<ButtonAttachment> (apv, //
                                                                 params.at (ParamNames::EnableAnalyzer),
                                                                 enableButton);

    inputSliderAttachment = std::make_unique<SliderAttachment> (apv,
                                                                params.at (AnalyzerProperties::ParamNames::AnalyzerDecayRate),
                                                                inputSlider);

    pointsSliderAttachment = std::make_unique<SliderAttachment> (apv,
                                                                 params.at (AnalyzerProperties::ParamNames::AnalyzerPoints),
                                                                 pointsSlider);

    decaySliderAttachment = std::make_unique<SliderAttachment> (apv,
                                                                params.at (AnalyzerProperties::ParamNames::AnalyzerDecayRate),
                                                                decaySlider);

    enableButton.setClickingTogglesState (true);
    enableButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::green);

    enableButton.setToggleState (true, juce::NotificationType::dontSendNotification);

    auto safePtr = juce::Component::SafePointer<AnalyzerControls> (this);

    enableButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
            comp->toggleEnable();
    };

    addAndMakeVisible (enableButton);
    addAndMakeVisible (inputSlider);
    addAndMakeVisible (pointsSlider);
    addAndMakeVisible (decaySlider);

    inputSlider.setSliderStyle (juce::Slider::SliderStyle::TwoValueVertical);
    inputSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);

    pointsSlider.setSliderStyle (juce::Slider::SliderStyle::ThreeValueVertical);
    pointsSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);

    decaySlider.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    decaySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
}

AnalyzerControls::~AnalyzerControls()
{
}

void AnalyzerControls::resized()
{
    auto bounds = getLocalBounds().reduced (10);

    auto padding = 10;
    auto sliderWidth = bounds.getWidth() / 3 - padding;
    auto sliderHeight = bounds.getHeight() - 20;

    enableButton.setBounds (bounds.removeFromLeft (20));
    inputSlider.setBounds (bounds.removeFromLeft (sliderWidth));
    pointsSlider.setBounds (bounds.removeFromLeft (sliderWidth));
    decaySlider.setBounds (bounds.removeFromLeft (sliderWidth));
}

void AnalyzerControls::toggleEnable()
{
    bool state = enableButton.getToggleState();
    inputSlider.setEnabled (state);
    pointsSlider.setEnabled (state);
    decaySlider.setEnabled (state);
    enableButton.setButtonText (state ? "On" : "Off");
}
