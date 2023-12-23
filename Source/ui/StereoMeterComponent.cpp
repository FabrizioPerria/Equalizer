#include "ui/StereoMeterComponent.h"

StereoMeterComponent::StereoMeterComponent (juce::String label)
{
    // TODO: add label
    addAndMakeVisible (leftMeter);
    addAndMakeVisible (rightMeter);
    addAndMakeVisible (dbScale);
}

void StereoMeterComponent::paint (juce::Graphics& g)
{
}

void StereoMeterComponent::resized()
{
    auto bounds = getLocalBounds();
    const auto meterWidth = 15;
    const auto scaleWidth = 20;
    const auto margin = 10;

    auto leftMeterBounds = bounds.removeFromLeft (meterWidth).withTrimmedTop (margin).withTrimmedBottom (margin);
    auto scaleBounds = bounds.removeFromLeft (scaleWidth);
    auto rightMeterBounds = bounds.removeFromLeft (meterWidth).withTrimmedTop (margin).withTrimmedBottom (margin);

#ifdef USE_TEST_OSC
    meterBounds.setY (JUCE_LIVE_CONSTANT (meterBounds.getY()));
    meterBounds.setHeight (JUCE_LIVE_CONSTANT (meterBounds.getHeight()));
#endif

    leftMeter.setBounds (leftMeterBounds);
    dbScale.setBounds (scaleBounds);
    rightMeter.setBounds (rightMeterBounds);

    dbScale.buildBackgroundImage (TICKS_INTERVAL, leftMeterBounds, NEGATIVE_INFINITY, MAX_DECIBELS);
}

void StereoMeterComponent::update (MeterValues meterValues)
{
    leftMeter.update (meterValues.leftPeakDb.getDb(), meterValues.leftRmsDb.getDb());
    rightMeter.update (meterValues.rightPeakDb.getDb(), meterValues.rightRmsDb.getDb());
}
