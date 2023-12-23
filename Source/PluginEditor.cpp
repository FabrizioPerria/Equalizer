/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "ui/MeterComponent.h"
#include "utils/EqParam.h"
#include "utils/MeterConstants.h"
//==============================================================================
EqualizerAudioProcessorEditor::EqualizerAudioProcessorEditor (EqualizerAudioProcessor& p) : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    addAndMakeVisible (inputMeter);
    addAndMakeVisible (inputScale);

    startTimerHz (FRAMES_PER_SECOND);
}

EqualizerAudioProcessorEditor::~EqualizerAudioProcessorEditor()
{
}

//==============================================================================
void EqualizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void EqualizerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    const auto meterWidth = 15;
    const auto scaleWidth = 20;
    const auto margin = 10;

    auto meterBounds = bounds.removeFromLeft (meterWidth).withTrimmedTop (margin).withTrimmedBottom (margin);
    auto scaleBounds = bounds.removeFromLeft (scaleWidth);

#ifdef USE_TEST_OSC
    meterBounds.setY (JUCE_LIVE_CONSTANT (meterBounds.getY()));
    meterBounds.setHeight (JUCE_LIVE_CONSTANT (meterBounds.getHeight()));
#endif

    inputScale.setBounds (scaleBounds);
    inputMeter.setBounds (meterBounds);

    inputScale.buildBackgroundImage (TICKS_INTERVAL, meterBounds, NEGATIVE_INFINITY, MAX_DECIBELS);
}

void EqualizerAudioProcessorEditor::timerCallback()
{
    while (audioProcessor.inputMeterFifo.getNumAvailableForReading() > 0)
    {
        auto success = audioProcessor.inputMeterFifo.pull (inputMeterBuffer);
        jassert (success);
    }

    auto gain = inputMeterBuffer.getMagnitude (static_cast<int> (Channel::LEFT), 0, inputMeterBuffer.getNumSamples());
    auto db = juce::Decibels::gainToDecibels (gain);
    inputMeter.update (db);
}
