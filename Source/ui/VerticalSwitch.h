#pragma once

#include "ui/EqControlLookAndFeel.h"
#include <JuceHeader.h>

struct VerticalSwitch : juce::Slider
{
    VerticalSwitch (juce::String newTitle)
        : Slider (juce::Slider::SliderStyle::LinearVertical, juce::Slider::TextEntryBoxPosition::NoTextBox), title (newTitle)
    {
        setLookAndFeel (&laf);
    }

    ~VerticalSwitch() override
    {
        setLookAndFeel (nullptr);
    }

    void paint (juce::Graphics& g) override
    {
        auto range = getRange();
        auto bounds = getLocalBounds();
        bounds.removeFromLeft (5);
        auto titleArea = bounds.removeFromTop (12);

        g.setColour (juce::Colours::aquamarine);
        g.setFont (12);
        g.drawFittedText (title, titleArea.toNearestInt(), juce::Justification::left, 1);

        bounds.removeFromTop (12);
        bounds.removeFromBottom (12);
        auto value = static_cast<float> (getValue());
        auto rangeStart = static_cast<float> (range.getStart());
        auto rangeEnd = static_cast<float> (range.getEnd());
        auto bottomBounds = static_cast<float> (bounds.getBottom());
        auto yBounds = static_cast<float> (bounds.getY());
        auto normalized = juce::jmap (value, rangeStart, rangeEnd, bottomBounds, yBounds);

        getLookAndFeel().drawLinearSlider (g,
                                           bounds.getX(),
                                           bounds.getY(),
                                           bounds.getWidth(),
                                           bounds.getHeight(),
                                           normalized,
                                           0,
                                           1,
                                           getSliderStyle(),
                                           *this);

        auto labelArea = bounds;

        labelArea.removeFromLeft (JUCE_LIVE_CONSTANT (12));
        labelArea.setHeight (10);
        g.setColour (juce::Colours::aquamarine);
        g.setFont (10);
        auto numChoices = labels.size();

        for (int i = 0; i < numChoices; ++i)
        {
            auto ypos = bounds.getBottom() - 5 - i * (bounds.getHeight() / (numChoices - 1));
            g.setColour (juce::Colours::green);
            g.drawFittedText (labels[i], labelArea.withY (ypos).toNearestInt(), juce::Justification::left, 1);
        }
    }

    juce::StringArray labels;

private:
    EqControlsLookAndFeel laf;

    juce::String title;

    juce::Rectangle<int> getSliderBounds() const
    {
        juce::Rectangle<int> r = getLocalBounds();

        return r;
    }
};
