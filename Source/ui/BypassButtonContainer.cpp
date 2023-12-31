#include "ui/BypassButtonContainer.h"

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

void GlobalBypassButton::paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = getLocalBounds().toFloat();
    juce::Path powerSymbol;
    g.setColour (juce::Colours::white);
    auto size = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.8f;

    auto r = bounds.withSizeKeepingCentre (size, size).toFloat();
    float angInRadians = juce::degreesToRadians (30.0f);
    //TODO: play with this
    size -= 6;

    powerSymbol.addCentredArc (r.getCentreX(),
                               r.getCentreY(),
                               size * 0.5f,
                               size * 0.5f,
                               0.0f,
                               angInRadians,
                               juce::MathConstants<float>::twoPi - angInRadians,
                               true);

    powerSymbol.startNewSubPath (r.getCentreX(), r.getY());
    powerSymbol.lineTo (bounds.getCentre());

    juce::PathStrokeType strokeType (2.0f);
    g.strokePath (powerSymbol, strokeType);
    g.drawRect (bounds);
}
