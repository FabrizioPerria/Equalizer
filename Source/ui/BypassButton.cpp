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
    const int margin = 3;
    if (isShowingAsOn())
    {
        auto bounds = getLocalBounds().toFloat();
        bounds.reduce (0, margin);

        if (isPaired)
        {
            if (onLeft)
            {
                bounds.removeFromLeft (margin);
            }
            else
            {
                bounds.removeFromRight (margin);
            }
        }
        else
        {
            bounds.reduce (margin, 0);
        }

        g.setColour (juce::Colours::green);
        g.fillRect (bounds);
    }
}
