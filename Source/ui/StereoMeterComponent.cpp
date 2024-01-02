#include "ui/StereoMeterComponent.h"
#include "utils/MeterConstants.h"

StereoMeterComponent::StereoMeterComponent (juce::String label)
{
    name = label;

    addAndMakeVisible (leftMeter);
    addAndMakeVisible (rightMeter);
    addAndMakeVisible (dbScale);
}

void StereoMeterComponent::paint (juce::Graphics& g)
{
    auto labelBounds = getLocalBounds().removeFromBottom (labelHeight);

    g.setColour (juce::Colours::aquamarine);
    g.drawFittedText (name, labelBounds, juce::Justification::centred, 1);
}

void StereoMeterComponent::resized()
{
    auto bounds = getLocalBounds();

    bounds.removeFromBottom (labelHeight);
    /* bounds.removeFromTop (componentsMargin); */

    auto leftMeterBounds = bounds.removeFromLeft (MONO_METER_WIDTH).withTrimmedBottom (componentsMargin);
    auto rightMeterBounds = bounds.removeFromRight (MONO_METER_WIDTH).withTrimmedBottom (componentsMargin);

    leftMeter.setBounds (leftMeterBounds);
    rightMeter.setBounds (rightMeterBounds);
    dbScale.setBounds (bounds);

    const auto dbScaleTicksInterval = 6;
    dbScale.buildBackgroundImage (dbScaleTicksInterval, leftMeter.getGaugeBounds(), NEGATIVE_INFINITY, MAX_DECIBELS);
}

void StereoMeterComponent::update (MeterValues meterValues)
{
    leftMeter.update (meterValues.leftPeakDb.getDb(), meterValues.leftRmsDb.getDb());
    rightMeter.update (meterValues.rightPeakDb.getDb(), meterValues.rightRmsDb.getDb());
}
