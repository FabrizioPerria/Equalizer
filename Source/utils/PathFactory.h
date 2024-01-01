#pragma once

#include <JuceHeader.h>

namespace PathFactory
{
inline juce::Path createPowerSymbol (const juce::Rectangle<float>& bounds)
{
    juce::Path powerSymbol;
    auto size = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.6f;

    auto r = bounds.withSizeKeepingCentre (size, size);
    float angInRadians = juce::degreesToRadians (30.0f);

    powerSymbol.addCentredArc (r.getCentreX(),
                               r.getCentreY(),
                               size / 2,
                               size / 2,
                               0.0f,
                               angInRadians,
                               juce::MathConstants<float>::twoPi - angInRadians,
                               true);

    powerSymbol.addRectangle (r.removeFromBottom (size / 2.0f));
    powerSymbol.addRectangle (r.removeFromTop (size / 2.0f));

    return powerSymbol;
}

} // namespace PathFactory
