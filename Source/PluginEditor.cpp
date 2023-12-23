/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "data/MeterValues.h"
#include "ui/MeterComponent.h"
#include "utils/EqParam.h"
#include "utils/MeterConstants.h"
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
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void EqualizerAudioProcessorEditor::resized()
{
    auto pluginBounds = getLocalBounds();
    //TODO: define margin and meter sizes here
    inputMeter.setBounds (pluginBounds.removeFromLeft (JUCE_LIVE_CONSTANT (80)));
    outputMeter.setBounds (pluginBounds.removeFromRight (JUCE_LIVE_CONSTANT (80 - 30)));
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
