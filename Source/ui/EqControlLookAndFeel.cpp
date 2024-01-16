#include "ui/EqControlLookAndFeel.h"
#include "ui/EqParamWidget.h"
#include "ui/KnobWithLabels.h"
#include "ui/VerticalSwitch.h"

void EqControlsLookAndFeel::drawRotarySlider (juce::Graphics& g,
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

        juce::ColourGradient gradient (light, centreX, centreY, dark, centreX + radius, centreY + radius, true);
        g.setGradientFill (gradient);
        g.fillEllipse (rx, ry, rw, rw);

        g.setColour (light.withAlpha (0.5f));
        g.drawEllipse (rx, ry, rw, rw, 1.0f);
        g.setColour (dark.withAlpha (0.5f));
        g.drawEllipse (rx + 2, ry + 2, rw - 4, rw - 4, 1.0f);

        juce::Path p;
        auto pointerLength = radius * 0.33f;
        auto pointerThickness = 2.0f;
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));

        g.setColour (light);
        g.fillPath (p);
    }
    else
    {
        juce::LookAndFeel_V4::drawRotarySlider (g, x, y, width, height, sliderPos, rotaryStartAngle, rotaryEndAngle, slider);
    }
}

void EqControlsLookAndFeel::drawLinearSlider (juce::Graphics& g,
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
    if (auto* textSlider = dynamic_cast<TextOnlyHorizontalSlider*> (&slider))
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
    else if (auto* sw = dynamic_cast<VerticalSwitch*> (&slider))
    {
        auto widthF = static_cast<float> (width);
        auto heightF = static_cast<float> (height);
        auto xF = static_cast<float> (x);
        auto yF = static_cast<float> (y);
        auto trackWidth = juce::jmin (8.0f, widthF / 4.0f);

        juce::Point<float> startPoint (xF + 5, heightF + yF);
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
    else
    {
        juce::LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
    }
}

void EqControlsLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                                  juce::Button& button,
                                                  const juce::Colour& backgroundColour,
                                                  bool shouldDrawButtonAsHighlighted,
                                                  bool shouldDrawButtonAsDown)
{
    g.setColour (button.getToggleState() ? juce::Colours::green : juce::Colours::black);

    auto bounds = button.getLocalBounds();
    g.fillRect (bounds);
    g.setColour (juce::Colours::white);
    g.drawRect (bounds, 1);
}