/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "utils/MeterConstants.h"

//==============================================================================
EqualizerAudioProcessorEditor::EqualizerAudioProcessorEditor (EqualizerAudioProcessor& p) : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);

    addAndMakeVisible (inputMeter);
    addAndMakeVisible (outputMeter);
    addAndMakeVisible (eqParamWidget0);
    addAndMakeVisible (eqParamWidget1);
    addAndMakeVisible (eqParamWidget2);
    addAndMakeVisible (eqParamWidget3);
    addAndMakeVisible (eqParamWidget4);
    addAndMakeVisible (eqParamWidget5);
    addAndMakeVisible (eqParamWidget6);
    addAndMakeVisible (eqParamWidget7);

    startTimerHz (FRAMES_PER_SECOND);
}

EqualizerAudioProcessorEditor::~EqualizerAudioProcessorEditor()
{
}

//==============================================================================
void EqualizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    auto pluginBounds = getLocalBounds().reduced (pluginMargin);
    g.setColour (juce::Colours::aquamarine);
    g.drawRoundedRectangle (pluginBounds.toFloat(), 10, 1);

    g.setColour (juce::Colour { 0x1A, 0x1B, 0x29 });
    g.fillRoundedRectangle (pluginBounds.toFloat(), 10);
}

void EqualizerAudioProcessorEditor::resized()
{
    auto pluginBounds = getLocalBounds().reduced (2 * pluginMargin, pluginMargin);
    auto stereoMeterWidth = MONO_METER_WIDTH + METER_SCALE_WIDTH + MONO_METER_WIDTH;
    inputMeter.setBounds (pluginBounds.removeFromLeft (stereoMeterWidth));
    outputMeter.setBounds (pluginBounds.removeFromRight (stereoMeterWidth));

    pluginBounds.reduce (2 * pluginMargin, 0);
    auto eqParamWidgetBounds = pluginBounds.removeFromBottom (84);
    auto eqParamWidgetWidth = eqParamWidgetBounds.getWidth() / 8;
    eqParamWidget0.setBounds (eqParamWidgetBounds.removeFromLeft (eqParamWidgetWidth));
    eqParamWidget1.setBounds (eqParamWidgetBounds.removeFromLeft (eqParamWidgetWidth));
    eqParamWidget2.setBounds (eqParamWidgetBounds.removeFromLeft (eqParamWidgetWidth));
    eqParamWidget3.setBounds (eqParamWidgetBounds.removeFromLeft (eqParamWidgetWidth));
    eqParamWidget4.setBounds (eqParamWidgetBounds.removeFromLeft (eqParamWidgetWidth));
    eqParamWidget5.setBounds (eqParamWidgetBounds.removeFromLeft (eqParamWidgetWidth));
    eqParamWidget6.setBounds (eqParamWidgetBounds.removeFromLeft (eqParamWidgetWidth));
    eqParamWidget7.setBounds (eqParamWidgetBounds.removeFromLeft (eqParamWidgetWidth));
}

void EqualizerAudioProcessorEditor::timerCallback()
{
    updateMeterValues (audioProcessor.inMeterValuesFifo, inputMeter);
    updateMeterValues (audioProcessor.outMeterValuesFifo, outputMeter);
}
