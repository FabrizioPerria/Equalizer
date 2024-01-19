#pragma once

#include "ui/AnalyzerWidgetBase.h"
#include "utils/ChainHelpers.h"
#include <JuceHeader.h>

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
