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
    auto labelBounds = getLocalBounds().removeFromBottom (METER_LABEL_TEXT_SIZE);

    g.setColour (juce::Colours::aquamarine);
    g.drawFittedText (name, labelBounds, juce::Justification::centred, 1);
}

void StereoMeterComponent::resized()
{
    auto bounds = getLocalBounds();

    bounds.removeFromBottom (METER_LABEL_TEXT_SIZE);

    auto leftMeterBounds = bounds.removeFromLeft (MONO_METER_WIDTH)
                               .withTrimmedTop (MONO_METER_Y_MARGIN)
                               .withTrimmedBottom (MONO_METER_Y_MARGIN);

    auto scaleBounds = bounds.removeFromLeft (SCALE_WIDTH).withTrimmedTop (METER_LABEL_TEXT_SIZE + 5);

    auto rightMeterBounds = bounds.removeFromLeft (MONO_METER_WIDTH)
                                .withTrimmedTop (MONO_METER_Y_MARGIN)
                                .withTrimmedBottom (MONO_METER_Y_MARGIN);

#ifdef USE_TEST_OSC
    meterBounds.setY (JUCE_LIVE_CONSTANT (meterBounds.getY()));
    meterBounds.setHeight (JUCE_LIVE_CONSTANT (meterBounds.getHeight()));
#endif

    leftMeter.setBounds (leftMeterBounds);
    dbScale.setBounds (scaleBounds);
    rightMeter.setBounds (rightMeterBounds);

    dbScale.buildBackgroundImage (TICKS_INTERVAL,
                                  rightMeterBounds.withTrimmedBottom (METER_LABEL_TEXT_SIZE + 5),
                                  NEGATIVE_INFINITY,
                                  MAX_DECIBELS);
}

void StereoMeterComponent::update (MeterValues meterValues)
{
    leftMeter.update (meterValues.leftPeakDb.getDb(), meterValues.leftRmsDb.getDb());
    rightMeter.update (meterValues.rightPeakDb.getDb(), meterValues.rightRmsDb.getDb());
}
