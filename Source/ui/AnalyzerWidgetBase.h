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
