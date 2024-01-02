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
        bounds.removeFromTop (2);
        bounds.removeFromBottom (2);

        if (isPaired)
        {
            if (onLeft)
            {
                bounds.removeFromLeft (2);
            }
            else
            {
                bounds.removeFromRight (2);
            }
        }
        else
        {
            //TODO: reduce??
            bounds.removeFromLeft (2);
            bounds.removeFromRight (2);
        }

        g.setColour (juce::Colours::green);
        g.fillRect (bounds);
    }
}
