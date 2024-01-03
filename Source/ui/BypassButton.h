#pragma once

#include <JuceHeader.h>

struct BypassButton : juce::Button
{
    BypassButton();
    bool isShowingAsOn() const;

    void paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    bool onLeft { false };
    bool isPaired { false };

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassButton)
};
