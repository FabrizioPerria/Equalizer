#include "ui/GlobalBypassButton.h"
#include "utils/PathDrawer.h"

GlobalBypassButton::GlobalBypassButton (EqualizerAudioProcessor& p) : audioProcessor (p)
{
    setClickingTogglesState (true);
    setToggleState (true, juce::dontSendNotification);
    startTimerHz (FRAMES_PER_SECOND);
}

void GlobalBypassButton::timerCallback()
{
    auto atLeastOneActive = audioProcessor.isAnyFilterActive();
    if (! atLeastOneActive && isShowingAsOn())
    {
        setToggleState (true, juce::dontSendNotification);
    }

    if (atLeastOneActive && ! isShowingAsOn())
    {
        setToggleState (false, juce::dontSendNotification);
    }
}

void GlobalBypassButton::clicked()
{
    audioProcessor.setGlobalBypass (! isShowingAsOn());
}

void GlobalBypassButton::paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
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
