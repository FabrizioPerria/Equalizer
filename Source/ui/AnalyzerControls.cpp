#include "ui/AnalyzerControls.h"
#include "utils/AnalyzerProperties.h"
#include <ratio>

void AnalyzerControlsLookAndFeel::drawRotarySlider (Graphics& g,
                                                    int x,
                                                    int y,
                                                    int width,
                                                    int height,
                                                    float sliderPos,
                                                    const float rotaryStartAngle,
                                                    const float rotaryEndAngle,
                                                    Slider& slider)
{
    if (auto* rswl = dynamic_cast<KnobWithLabels*> (&slider))
    {
        auto radius = jmin (width / 2, height / 2) - 4.0f;
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // fill
        ColourGradient gradient (light, centreX, centreY, dark, centreX + radius, centreY + radius, true);
        g.setGradientFill (gradient);
        g.fillEllipse (rx, ry, rw, rw);

        // 3D effect
        g.setColour (light.withAlpha (0.5f));
        g.drawEllipse (rx, ry, rw, rw, 1.0f);
        g.setColour (dark.withAlpha (0.5f));
        g.drawEllipse (rx + 2, ry + 2, rw - 4, rw - 4, 1.0f);

        // pointer
        Path p;
        auto pointerLength = radius * 0.33f;
        auto pointerThickness = 2.0f;
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (AffineTransform::rotation (angle).translated (centreX, centreY));

        // pointer color
        g.setColour (light);
        g.fillPath (p);
    }
}

void AnalyzerControlsLookAndFeel::drawLinearSlider (Graphics& g,
                                                    int x,
                                                    int y,
                                                    int width,
                                                    int height,
                                                    float sliderPos,
                                                    float minSliderPos,
                                                    float maxSliderPos,
                                                    const Slider::SliderStyle style,
                                                    Slider& slider)
{
    auto trackWidth = jmin (8.0f, slider.isHorizontal() ? height * 0.25f : width * 0.25f);

    Point<float> startPoint (slider.isHorizontal() ? x : x + width * 0.5f, //
                             slider.isHorizontal() ? y + height * 0.5f : height + y);

    Point<float> endPoint (slider.isHorizontal() ? width + x : startPoint.x, //
                           slider.isHorizontal() ? startPoint.y : y);

    Path backgroundTrack;
    backgroundTrack.startNewSubPath (startPoint);
    backgroundTrack.lineTo (endPoint);
    g.setColour (darkdark.withAlpha (0.8f));
    g.strokePath (backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

    g.setColour (light);
    auto pointer = Rectangle<float> (trackWidth, trackWidth);
    pointer.setCentre ({ slider.isHorizontal() ? sliderPos : startPoint.x, slider.isHorizontal() ? startPoint.y : sliderPos });

    g.fillEllipse (pointer);
}

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

    addAndMakeVisible (enableButton);
    addAndMakeVisible (inputSlider);
    addAndMakeVisible (pointsSlider);
    addAndMakeVisible (decaySlider);

    inputSlider.setSliderStyle (juce::Slider::SliderStyle::LinearVertical);
    inputSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 50, 20);

    pointsSlider.setSliderStyle (juce::Slider::SliderStyle::LinearVertical);
    pointsSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 50, 20);

    decaySlider.labels.add ("0");
    decaySlider.labels.add ("30");
}

AnalyzerControls::~AnalyzerControls()
{
    setLookAndFeel (nullptr);
}

void AnalyzerControls::resized()
{
    auto bounds = getLocalBounds().reduced (10);

    auto padding = 10;
    auto sliderWidth = bounds.getWidth() / 5 - padding;
    auto sliderHeight = bounds.getHeight() - 20;

    enableButton.setBounds (bounds.removeFromLeft (50));
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
