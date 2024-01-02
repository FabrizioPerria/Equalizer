#include "PluginProcessor.h"
#include "ui/BypassButton.h"
#include "utils/PathFactory.h"
#include <JuceHeader.h>

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

    void paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour (juce::Colours::red);

        if (isShowingAsOn())
        {
            g.setColour (juce::Colours::green);
        }
        PathDrawer::drawPowerSymbol (g, bounds);
        g.drawRect (bounds);
    }

private:
    EqualizerAudioProcessor& audioProcessor;
};
