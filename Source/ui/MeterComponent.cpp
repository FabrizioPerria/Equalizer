#include "ui/MeterComponent.h"
#include <JuceHeader.h>

void MeterComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    auto meterRect = getLocalBounds().toFloat();

    const float meterHeight = meterRect.getHeight();

    auto meterColor = juce::Colours::green;

    if (peakDb > NEGATIVE_INFINITY)
    {
        auto relativePeak = juce::jmap (peakDb, NEGATIVE_INFINITY, MAX_DECIBELS, meterHeight, 0.0f);
        auto meterFill = meterRect.withY (relativePeak).withBottom (meterRect.getBottom());
        g.setColour (meterColor);
        g.fillRect (meterFill);
    }

    auto tickPeak = juce::jmap (peakDbDecay.getCurrentValue(), NEGATIVE_INFINITY, MAX_DECIBELS, meterHeight, 0.0f);
    auto peakLine = juce::Line<float> (meterRect.getX(), tickPeak, meterRect.getRight(), tickPeak);

    g.setColour (peakDbDecay.isOverThreshold() ? juce::Colours::red : juce::Colours::orange);
    g.drawLine (peakLine, 2.0f);
}

void MeterComponent::update (float dbLevel)
{
    peakDb = dbLevel;
    peakDbDecay.updateHeldValue (dbLevel);
    repaint();
}
