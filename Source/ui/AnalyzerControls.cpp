#include "ui/AnalyzerControls.h"
#include "utils/AnalyzerProperties.h"

AnalyzerControls::AnalyzerControls (juce::AudioProcessorValueTreeState& apv)
{
    using ParamNames = AnalyzerProperties::ParamNames;
    setLookAndFeel (&laf);

    auto params = AnalyzerProperties::GetAnalyzerParams();
    enableButtonAttachment = std::make_unique<ButtonAttachment> (apv, //
                                                                 params.at (ParamNames::EnableAnalyzer),
                                                                 enableButton);

    inputSliderAttachment = std::make_unique<SliderAttachment> (apv,
                                                                params.at (AnalyzerProperties::ParamNames::AnalyzerProcessingMode),
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

    enableTitle.setText ("Analyzer", juce::dontSendNotification);
    enableTitle.setFont (12);
    enableTitle.setColour (juce::Label::textColourId, juce::Colours::aquamarine);
    addAndMakeVisible (enableTitle);
    addAndMakeVisible (enableButton);
    addAndMakeVisible (inputSlider);
    addAndMakeVisible (pointsSlider);
    addAndMakeVisible (decaySlider);

    decaySlider.labels.add ("0");
    decaySlider.labels.add ("30");

    inputSlider.labels.add ("Pre");
    inputSlider.labels.add ("Post");

    pointsSlider.labels.add ("2048");
    pointsSlider.labels.add ("4096");
    pointsSlider.labels.add ("8192");
}

AnalyzerControls::~AnalyzerControls()
{
    setLookAndFeel (nullptr);
}

void AnalyzerControls::resized()
{
    auto bounds = getLocalBounds().reduced (10, 0);

    auto padding = 10;
    auto sliderWidth = bounds.getWidth() / 5 - padding;

    auto enableButtonArea = bounds.removeFromLeft (50);
    enableTitle.setBounds (enableButtonArea.removeFromTop (12));
    enableButtonArea.reduce (0, 8);
    enableButton.setBounds (enableButtonArea);
    bounds.removeFromLeft (40);
    decaySlider.setBounds (bounds.removeFromRight (80));
    bounds.removeFromRight (30);
    inputSlider.setBounds (bounds.removeFromLeft (sliderWidth));
    bounds.removeFromLeft (30);
    pointsSlider.setBounds (bounds.removeFromLeft (sliderWidth));
}

void AnalyzerControls::toggleEnable()
{
    bool state = enableButton.getToggleState();
    inputSlider.setEnabled (state);
    pointsSlider.setEnabled (state);
    decaySlider.setEnabled (state);
    enableButton.setButtonText (state ? "On" : "Off");
}
