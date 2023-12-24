/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "data/MeterValues.h"
#include "utils/MeterConstants.h"

#define PLUGIN_MARGIN 5

//==============================================================================
EqualizerAudioProcessorEditor::EqualizerAudioProcessorEditor (EqualizerAudioProcessor& p) : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);

    addAndMakeVisible (inputMeter);
    addAndMakeVisible (outputMeter);

    startTimerHz (FRAMES_PER_SECOND);
}

EqualizerAudioProcessorEditor::~EqualizerAudioProcessorEditor()
{
}

//==============================================================================
void EqualizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    auto pluginBounds = getLocalBounds().reduced (PLUGIN_MARGIN);
    g.setColour (juce::Colours::aquamarine);
    g.drawRoundedRectangle (pluginBounds.toFloat(), 10, 1);

    g.setColour (juce::Colour { 0x1A, 0x1B, 0x29 });
    g.fillRoundedRectangle (pluginBounds.toFloat(), 10);
}

void EqualizerAudioProcessorEditor::resized()
{
    auto pluginBounds = getLocalBounds().reduced (PLUGIN_MARGIN + STEREO_METER_MARGIN, PLUGIN_MARGIN);
    auto stereoMeterWidth = MONO_METER_WIDTH + STEREO_METER_SCALE_WIDTH + MONO_METER_WIDTH;
    inputMeter.setBounds (pluginBounds.removeFromLeft (stereoMeterWidth));
    outputMeter.setBounds (pluginBounds.removeFromRight (stereoMeterWidth));
}

void EqualizerAudioProcessorEditor::timerCallback()
{
    //TODO: refactor this
    if (audioProcessor.inMeterValuesFifo.getNumAvailableForReading() > 0)
    {
        MeterValues inputMeterValues;
        while (audioProcessor.inMeterValuesFifo.getNumAvailableForReading() > 0)
        {
            auto success = audioProcessor.inMeterValuesFifo.pull (inputMeterValues);
            jassert (success);
        }

        inputMeter.update (inputMeterValues);
    }

    if (audioProcessor.outMeterValuesFifo.getNumAvailableForReading() > 0)
    {
        MeterValues outputMeterValues;
        while (audioProcessor.outMeterValuesFifo.getNumAvailableForReading() > 0)
        {
            auto success = audioProcessor.outMeterValuesFifo.pull (outputMeterValues);
            jassert (success);
        }

        outputMeter.update (outputMeterValues);
    }
}
