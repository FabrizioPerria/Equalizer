#include "ui/MeterComponent.h"
#include <JuceHeader.h>

void MeterComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    auto meterRect = getLocalBounds().toFloat();

    if (peakDb > NEGATIVE_INFINITY)
    {
        paintRectangleForValue (g, peakDb, meterRect, juce::Colours::green);
    }

    paintRectangleForValue (g, averageDb.getAvg(), meterRect.withTrimmedLeft (5).withTrimmedRight (5), juce::Colours::gold);

    auto tickPeak = juce::jmap (peakDbDecay.getCurrentValue(), NEGATIVE_INFINITY, MAX_DECIBELS, meterRect.getHeight(), 0.0f);
    auto peakLine = juce::Line<float> (meterRect.getX(), tickPeak, meterRect.getRight(), tickPeak);
    g.setColour (peakDbDecay.isOverThreshold() ? juce::Colours::red : juce::Colours::orange);
    g.drawLine (peakLine, 2.0f);
}

void MeterComponent::paintRectangleForValue (juce::Graphics& g, float value, juce::Rectangle<float> rect, juce::Colour color)
{
    auto relativeValue = juce::jmap (value, NEGATIVE_INFINITY, MAX_DECIBELS, rect.getHeight(), 0.0f);
    auto fill = rect.withY (relativeValue).withBottom (rect.getBottom());
    g.setColour (color);
    g.fillRect (fill);
}

void MeterComponent::update (float dbLevel)
{
    peakDb = dbLevel;
    peakDbDecay.updateHeldValue (dbLevel);
    averageDb.add (dbLevel);
    repaint();
}
