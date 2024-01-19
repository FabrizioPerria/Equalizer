#pragma once

#include "ui/AnalyzerWidgetBase.h"
#include "utils/ChainHelpers.h"
#include <JuceHeader.h>

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
