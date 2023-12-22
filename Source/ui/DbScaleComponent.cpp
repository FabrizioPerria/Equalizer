#include "ui/DbScaleComponent.h"
#include <JuceHeader.h>

void DbScaleComponent::paint (juce::Graphics& g)
{
    g.drawImage (bkgd, getLocalBounds().toFloat());
}

void DbScaleComponent::buildBackgroundImage (int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb)
{
    if (minDb > maxDb)
    {
        return;
    }

    auto bounds = getLocalBounds();
    if (bounds.isEmpty())
    {
        return;
    }

    auto desktopScaleFactor = juce::Desktop::getInstance().getGlobalScaleFactor();
    auto height = bounds.getHeight() * desktopScaleFactor;
    auto width = bounds.getWidth() * desktopScaleFactor;
    bkgd = juce::Image (juce::Image::RGB, width, height, true);

    auto g = juce::Graphics (bkgd);
    g.addTransform (juce::AffineTransform::scale (desktopScaleFactor));
    g.setColour (juce::Colours::lightgrey);

    auto ticks = getTicks (dbDivision, meterBounds, minDb, maxDb);

    const int textHeight = 14;
    for (auto& tick : ticks)
    {
        auto tickBounds = juce::Rectangle<int> { bounds.getX(), tick.y - textHeight / 2, bounds.getWidth(), textHeight };
        g.drawFittedText (juce::String (tick.db), tickBounds, juce::Justification::centred, 1);
    }
}

std::vector<Tick> DbScaleComponent::getTicks (int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb)
{
    if (minDb > maxDb)
    {
        std::swap (minDb, maxDb);
    }

    size_t numTicks = static_cast<size_t> ((maxDb - minDb) / dbDivision);
    std::vector<Tick> ticks;
    ticks.reserve (numTicks);
    for (int db = minDb; db <= maxDb; db += dbDivision)
    {
        Tick tick;
        tick.db = db;
        tick.y = juce::jmap (db, minDb, maxDb, meterBounds.getBottom(), meterBounds.getY());
        ticks.push_back (tick);
    }

    return ticks;
}
