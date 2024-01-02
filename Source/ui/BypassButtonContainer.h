#pragma once

#include "ui/DualBypassButton.h"
#include <JuceHeader.h>

struct BypassButtonContainer : juce::Component
{
    BypassButtonContainer (juce::AudioProcessorValueTreeState& apvts);

    void resized() override;

private:
    std::array<std::unique_ptr<DualBypassButton>, 8> bypassButtons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassButtonContainer)
};
