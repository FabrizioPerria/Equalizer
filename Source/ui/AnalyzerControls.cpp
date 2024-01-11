#include "ui/AnalyzerControls.h"
#include "utils/AnalyzerProperties.h"

void AnalyzerControlsLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                                    int x,
                                                    int y,
                                                    int width,
                                                    int height,
                                                    float sliderPos,
                                                    const float rotaryStartAngle,
                                                    const float rotaryEndAngle,
                                                    juce::Slider& slider)
{
    if (auto* rswl = dynamic_cast<KnobWithLabels*> (&slider))
    {
        auto widthF = static_cast<float> (width);
        auto heightF = static_cast<float> (height);
        auto xF = static_cast<float> (x);
        auto yF = static_cast<float> (y);
        auto radius = juce::jmin (widthF / 2, heightF / 2) - 4.0f;
        auto centreX = xF + widthF * 0.5f;
        auto centreY = yF + heightF * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // fill
        juce::ColourGradient gradient (light, centreX, centreY, dark, centreX + radius, centreY + radius, true);
        g.setGradientFill (gradient);
        g.fillEllipse (rx, ry, rw, rw);

        // 3D effect
        g.setColour (light.withAlpha (0.5f));
        g.drawEllipse (rx, ry, rw, rw, 1.0f);
        g.setColour (dark.withAlpha (0.5f));
        g.drawEllipse (rx + 2, ry + 2, rw - 4, rw - 4, 1.0f);

        // pointer
        juce::Path p;
        auto pointerLength = radius * 0.33f;
        auto pointerThickness = 2.0f;
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));

        // pointer color
        g.setColour (light);
        g.fillPath (p);
    }
}

void AnalyzerControlsLookAndFeel::drawLinearSlider (juce::Graphics& g,
                                                    int x,
                                                    int y,
                                                    int width,
                                                    int height,
                                                    float sliderPos,
                                                    float minSliderPos,
                                                    float maxSliderPos,
                                                    const juce::Slider::SliderStyle style,
                                                    juce::Slider& slider)
{
    if (auto* sw = dynamic_cast<VerticalSwitch*> (&slider))
    {
        auto widthF = static_cast<float> (width);
        auto heightF = static_cast<float> (height);
        auto xF = static_cast<float> (x);
        auto yF = static_cast<float> (y);
        auto trackWidth = juce::jmin (8.0f, widthF / 4.0f);

        juce::Point<float> startPoint (xF + JUCE_LIVE_CONSTANT (5), heightF + yF);
        /* juce::Point<float> startPoint (xF + widthF * 0.5f, heightF + yF); */
        juce::Point<float> endPoint (startPoint.x, yF);

        juce::Path backgroundTrack;
        backgroundTrack.startNewSubPath (startPoint);
        backgroundTrack.lineTo (endPoint);
        g.setColour (darkdark.withAlpha (0.8f));
        g.strokePath (backgroundTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

        g.setColour (light);
        auto pointer = juce::Rectangle<float> (trackWidth, trackWidth);
        pointer.setCentre ({ startPoint.x, sliderPos });

        g.fillEllipse (pointer);
    }
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

    enableButton.setBounds (bounds.removeFromLeft (50));
    bounds.removeFromLeft (30);
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
