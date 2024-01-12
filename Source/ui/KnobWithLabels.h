#pragma once

#include "ui/EqControlLookAndFeel.h"
#include <JuceHeader.h>

struct KnobWithLabels : juce::Slider
{
    KnobWithLabels (juce::String newTitle)
        : Slider (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox), title (newTitle)
    {
        setLookAndFeel (&lnf);
    }
    ~KnobWithLabels() override
    {
        setLookAndFeel (nullptr);
    }

    void paint (juce::Graphics& g) override
    {
        auto startAng = juce::degreesToRadians (180.f + 45.f);
        auto endAng = juce::degreesToRadians (180.f - 45.f) + juce::MathConstants<float>::twoPi;

        auto range = getRange();

        auto bounds = getLocalBounds();
        auto titleArea = bounds.removeFromTop (getTextHeight());
        auto sliderBounds = bounds;
        g.setColour (juce::Colours::aquamarine);
        g.setFont (getTextHeight());
        g.drawFittedText (title, titleArea, juce::Justification::centred, 1);
        sliderBounds.removeFromTop (12);
        sliderBounds.removeFromBottom (12);

        auto value = static_cast<float> (getValue());
        auto rangeStart = static_cast<float> (range.getStart());
        auto rangeEnd = static_cast<float> (range.getEnd());
        getLookAndFeel().drawRotarySlider (g,
                                           sliderBounds.getX(),
                                           sliderBounds.getY(),
                                           sliderBounds.getWidth(),
                                           sliderBounds.getHeight(),
                                           juce::jmap (value, rangeStart, rangeEnd, 0.0f, 1.0f),
                                           startAng,
                                           endAng,
                                           *this);

        auto center = sliderBounds.toFloat().getCentre();

        g.setColour (juce::Colour (0u, 172u, 1u));
        g.setFont (getTextHeight());

        auto numChoices = labels.size();
        for (int i = 0; i < numChoices; ++i)
        {
            auto pos = i * (1.f / (numChoices - 1));
            jassert (0.f <= pos);
            jassert (pos <= 1.f);

            auto ang = juce::jmap (pos, 0.f, 1.f, startAng, endAng);

            auto c = center.getPointOnCircumference (35, ang);

            juce::Rectangle<float> r;
            auto str = labels[i];
            r.setSize (g.getCurrentFont().getStringWidth (str), getTextHeight());
            r.setCentre (c);
            r.setY (r.getY());

            g.drawFittedText (str, r.toNearestInt(), juce::Justification::centred, 1);
        }
    }

    juce::Array<juce::String> labels;

    juce::Rectangle<int> getSliderBounds() const
    {
        auto bounds = getLocalBounds();

        auto size = juce::jmin (bounds.getWidth(), bounds.getHeight());

        size -= getTextHeight() * 2;
        juce::Rectangle<int> r;
        r.setSize (size, size);
        r.setCentre (bounds.getCentreX(), 0);
        r.setY (2);

        return r;
    }

    int getTextHeight() const
    {
        return 12;
    }

private:
    EqControlsLookAndFeel lnf;
    juce::String title;
};
