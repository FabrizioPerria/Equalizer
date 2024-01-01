#include <ui/BypassButton.h>

BypassButton::BypassButton() : juce::Button ("")
{
    setClickingTogglesState (true);
}

bool BypassButton::isShowingAsOn() const
{
    return ! getToggleState();
}

void BypassButton::paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    if (isShowingAsOn())
    {
        auto bounds = getLocalBounds().toFloat();
        bounds.removeFromTop (5);
        bounds.removeFromBottom (5);

        if (isPaired)
        {
            if (onLeft)
            {
                bounds.removeFromLeft (5);
            }
            else
            {
                bounds.removeFromRight (5);
            }
        }
        else
        {
            //TODO: reduce??
            bounds.removeFromLeft (5);
            bounds.removeFromRight (5);
        }

        g.setColour (juce::Colours::green);
        g.fillRect (bounds);
    }
}
