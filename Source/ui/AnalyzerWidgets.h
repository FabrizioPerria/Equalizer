#pragma once

#include "utils/ChainHelpers.h"
#include <JuceHeader.h>

struct AnalyzerWidgetBase : juce::Component
{
    AnalyzerWidgetBase (ChainPositions cp, Channel ch) : chainPosition (cp), channel (ch)
    {
    }

    void displayAsSelected (bool selected)
    {
        isSelected = selected;
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (isSelected ? juce::Colours::red : juce::Colours::blue);
        g.fillEllipse (getLocalBounds().toFloat());
    }
    ChainPositions getChainPosition() const
    {
        return chainPosition;
    }
    Channel getChannel() const
    {
        return channel;
    }

protected:
    bool isSelected = false;
    /*
     in order to know what param you should be controlling, you need to know the chainPos and channel you're associated with.
     */
    ChainPositions chainPosition;
    Channel channel;
};

struct AnalyzerBand : AnalyzerWidgetBase
{
    AnalyzerBand (ChainPositions cp, Channel ch) : AnalyzerWidgetBase (cp, ch)
    {
    }

    juce::Colour getColor (ChainPositions cp, Channel ch) const
    {
        auto numBands = static_cast<float> (std::numeric_limits<ChainPositions>::max());
        float bandPosition = static_cast<float> (cp) / numBands;
        switch (ch)
        {
            case Channel::LEFT:
                return juce::Colour::fromHSV (juce::jmap (bandPosition, 0.1f, 0.4f), 0.8f, 1.0f, 1.0f);
            case Channel::RIGHT:
                return juce::Colour::fromHSV (juce::jmap (bandPosition, 0.5f, 0.8f), 0.8f, 1.0f, 1.0f);
        }
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (getColor (chainPosition, channel));
        g.setOpacity (isSelected ? 0.5f : 0.0f);
        g.fillRect (getLocalBounds());
    }
};

struct AnalyzerNode : AnalyzerWidgetBase
{
    AnalyzerNode (ChainPositions cp, Channel ch) : AnalyzerWidgetBase (cp, ch)
    {
        setSize (8, 8);
    }
    juce::Rectangle<int> mouseOverBounds, qualityBounds;
    float getFrequency() const
    {
        return frequency;
    }
    float getGainOrSlope() const
    {
        return gainOrSlope;
    }
    /** returns true if the frequency was updated */
    bool updateFrequency (float f)
    {
        return update (frequency, f);
    }
    /** returns true if the gain/slope was updated */
    bool updateGainOrSlope (float g)
    {
        return update (gainOrSlope, g);
    }

private:
    float frequency = 0.f;
    float gainOrSlope = NEGATIVE_INFINITY;

    bool update (float& oldVal, float newVal)
    {
        if (oldVal == newVal)
            return false;

        oldVal = newVal;
        return true;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyzerNode)
};

struct AnalyzerQControl : AnalyzerWidgetBase
{
    AnalyzerQControl() : AnalyzerWidgetBase (ChainPositions::PEAK1, Channel::LEFT)
    {
        setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
    }

    void setChainPosition (ChainPositions cp)
    {
        chainPosition = cp;
    }

    void setChannel (Channel ch)
    {
        channel = ch;
    }
};
