#include "ui/MeterComponent.h"
#include "utils/MeterConstants.h"
#include <JuceHeader.h>

MeterComponent::MeterComponent (juce::String label)
{
    name = label;
}

void MeterComponent::resized()
{
    buildForegroundImage();
}

void MeterComponent::paint (juce::Graphics& g)
{
    auto meterRect = getLocalBounds().toFloat();

    auto labelRect = meterRect.removeFromTop (labelHeight);
    g.setColour (peakDb > 0 ? juce::Colours::red : juce::Colours::white);
    g.setFont (labelHeight);
    g.drawFittedText (name, labelRect.toNearestInt(), juce::Justification::centred, 1);

    meterRect.removeFromTop (labelMargin);

    auto meterBase = meterRect.getBottom();
    auto meterX = meterRect.getX();
    auto meterY = meterRect.getY();
    auto meterRight = meterRect.getRight();

    g.reduceClipRegion (meterRect.toNearestInt());
    if (peakDb > NEGATIVE_INFINITY)
    {
        paintRectangleForValue (g, peakDb, meterRect, juce::Colours::green);
    }

    paintRectangleForValue (g, averageDb.getAvg(), meterRect.reduced (5, 0), juce::Colours::gold);

    auto tickPeak = juce::jmap (peakDbDecay.getCurrentValue(), NEGATIVE_INFINITY, MAX_DECIBELS, meterBase, meterY);
    auto peakLine = juce::Line<float> (meterX, tickPeak, meterRight, tickPeak);
    g.setColour (peakDbDecay.isOverThreshold() ? juce::Colours::red : juce::Colours::white);
    g.drawLine (peakLine, 2.0f);

    g.drawImage (bkgd, meterRect);
}

juce::Rectangle<int> MeterComponent::getGaugeBounds() const
{
    return getLocalBounds().withTrimmedTop (static_cast<int> (labelHeight + labelMargin));
}

void MeterComponent::paintRectangleForValue (juce::Graphics& g, float value, juce::Rectangle<float> rect, juce::Colour color)
{
    auto rectBase = rect.getBottom();
    auto relativeValue = juce::jmap (value, NEGATIVE_INFINITY, MAX_DECIBELS, rectBase, rect.getY());
    auto fill = rect.withY (relativeValue).withBottom (rectBase);
    g.setColour (color);
    g.fillRect (fill);
}

void MeterComponent::update (float peakDbLevel, float rmsDbLevel)
{
    peakDb = peakDbLevel;
    peakDbDecay.updateHeldValue (peakDbLevel);
    averageDb.add (rmsDbLevel);
    repaint();
}

void MeterComponent::buildForegroundImage()
{
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

    auto meterRect = bounds.toFloat();
    auto meterBase = meterRect.getBottom();
    auto meterX = meterRect.getX();
    auto meterY = meterRect.getY();
    auto meterRight = meterRect.getRight();

    g.setColour (juce::Colours::darkgrey);
    g.drawRect (meterRect);

    for (auto i = NEGATIVE_INFINITY; i < MAX_DECIBELS; i += 12)
    {
        auto tickY = juce::jmap (static_cast<float> (i), NEGATIVE_INFINITY, MAX_DECIBELS, meterBase, meterY);
        auto widthReduction = static_cast<int> (i) == 0 ? 3.f : 5.f;
        auto tickLine = juce::Line<float> (meterX + widthReduction, tickY, meterRight - widthReduction, tickY);
        g.drawLine (tickLine, 2.0f);
    }
}
