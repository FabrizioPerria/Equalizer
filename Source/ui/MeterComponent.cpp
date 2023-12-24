#include "ui/MeterComponent.h"
#include <JuceHeader.h>

MeterComponent::MeterComponent (juce::String label)
{
    name = label;
}

void MeterComponent::paint (juce::Graphics& g)
{
    auto meterRect = getLocalBounds().toFloat();
    auto labelRect = meterRect.removeFromTop (METER_LABEL_TEXT_SIZE).toNearestInt();

    g.setColour (juce::Colours::darkgrey);
    g.drawRect (meterRect);

    if (peakDb > NEGATIVE_INFINITY)
    {
        paintRectangleForValue (g, peakDb, meterRect, juce::Colours::green);
    }

    auto avg = averageDb.getAvg();
    paintRectangleForValue (g, avg, meterRect.withTrimmedLeft (5).withTrimmedRight (5), juce::Colours::gold);

    auto tickPeak = juce::jmap (peakDbDecay.getCurrentValue(), NEGATIVE_INFINITY, MAX_DECIBELS, meterRect.getBottom(), meterRect.getY());
    auto peakLine = juce::Line<float> (meterRect.getX(), tickPeak, meterRect.getRight(), tickPeak);

    g.setColour (peakDbDecay.isOverThreshold() ? juce::Colours::red : juce::Colours::orange);

    g.drawLine (peakLine, 2.0f);
    g.drawFittedText (name, labelRect, juce::Justification::centred, 1);
}

void MeterComponent::paintRectangleForValue (juce::Graphics& g, float value, juce::Rectangle<float> rect, juce::Colour color)
{
    auto relativeValue = juce::jmap (value, NEGATIVE_INFINITY, MAX_DECIBELS, rect.getBottom(), rect.getY());
    auto fill = rect.withY (relativeValue).withBottom (rect.getBottom());
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
