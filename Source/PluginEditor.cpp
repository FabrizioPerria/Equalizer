/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "ui/MeterComponent.h"
#include "utils/EqParam.h"
//==============================================================================
EqualizerAudioProcessorEditor::EqualizerAudioProcessorEditor (EqualizerAudioProcessor& p) : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    addAndMakeVisible (inputMeter);
    addAndMakeVisible (inputScale);

    startTimerHz (60);
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
    const auto scaleWidth = 15;
    const auto margin = 10;

    auto wholeMeterBounds = bounds.removeFromLeft (meterWidth + scaleWidth);
    auto meterBounds = wholeMeterBounds.withTrimmedTop (margin).withTrimmedBottom (margin).removeFromLeft (scaleWidth);
    auto scaleBounds = wholeMeterBounds.withX (meterWidth);

#ifdef USE_TEST_OSC
    meterBounds.setY (JUCE_LIVE_CONSTANT (meterBounds.getY()));
    meterBounds.setHeight (JUCE_LIVE_CONSTANT (meterBounds.getHeight()));
#endif

    inputScale.setBounds (scaleBounds);
    inputMeter.setBounds (meterBounds);

    inputScale.buildBackgroundImage (6, meterBounds, NEGATIVE_INFINITY, MAX_DECIBELS);
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
