#include "ui/DbScaleComponent.h"
#include "utils/MeterConstants.h"
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
    auto scaledHeight = bounds.getHeight() * desktopScaleFactor;
    auto scaledWidth = bounds.getWidth() * desktopScaleFactor;
    bkgd = juce::Image (juce::Image::RGB, scaledWidth, scaledHeight, true);

    auto g = juce::Graphics (bkgd);

    g.addTransform (juce::AffineTransform::scale (desktopScaleFactor));
    g.setColour (juce::Colours::darkgrey);

    g.setFont (SCALE_TEXT_SIZE);
    auto ticks = getTicks (dbDivision, meterBounds, minDb, maxDb);

    for (auto& tick : ticks)
    {
        auto tickBounds = bounds;
        tickBounds.setY (tick.y - SCALE_TEXT_SIZE / 2.f);
        tickBounds.setHeight (SCALE_TEXT_SIZE);
        g.drawFittedText (tick.displayText, tickBounds, juce::Justification::centred, 1);
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
        tick.db = static_cast<float> (db);
        tick.y = juce::jmap (db, minDb, maxDb, meterBounds.getBottom(), meterBounds.getY());
        if (db <= 0)
        {
            tick.displayText = juce::String (db);
        }
        else
        {
            tick.displayText = juce::String ("+") + juce::String (db);
        }
        ticks.push_back (tick);
    }

    return ticks;
}
