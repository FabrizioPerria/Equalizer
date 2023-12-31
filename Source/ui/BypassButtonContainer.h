#pragma once

#include <JuceHeader.h>

struct BypassButton : juce::Button
{
    BypassButton() : juce::Button ("BypassButton")
    {
    }
    bool isShowingAsOn() const;

    void paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
    bool onLeft { false };
    bool isPaired { false };
};

struct GlobalBypassButton : BypassButton
{
    GlobalBypassButton()
    {
    }
    void paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
};

struct DualBypassButton : juce::Component
{
private:
};

struct BypassButtonContainer : juce::Component
{
private:
};
