#pragma once

#include "ui/AnalyzerWidgetBase.h"
#include "utils/ChainHelpers.h"
#include <JuceHeader.h>

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
