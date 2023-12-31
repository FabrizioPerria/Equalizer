#pragma once

#include "PluginProcessor.h"
#include <JuceHeader.h>

struct BypassButton : juce::Button
{
    BypassButton() : juce::Button ("")
    {
    }
    bool isShowingAsOn() const;

    void paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
    bool onLeft { false };
    bool isPaired { false };
};

struct GlobalBypassButton : BypassButton, juce::Timer
{
    GlobalBypassButton (EqualizerAudioProcessor& p) : audioProcessor (p)
    {
        setClickingTogglesState (true);
        setToggleState (true, juce::dontSendNotification);
        startTimerHz (FRAMES_PER_SECOND);
    }

    void timerCallback() override
    {
        auto atLeastOneActive = audioProcessor.isAnyFilterActive();
        if (! atLeastOneActive && isShowingAsOn())
        {
            setToggleState (false, juce::dontSendNotification);
        }

        if (atLeastOneActive && ! isShowingAsOn())
        {
            setToggleState (true, juce::dontSendNotification);
        }
    }

    void clicked() override
    {
        audioProcessor.setGlobalBypass (! isShowingAsOn());
    }

    void paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
    EqualizerAudioProcessor& audioProcessor;
};

struct DualBypassButton : juce::Component
{
private:
};

struct BypassButtonContainer : juce::Component
{
private:
};
