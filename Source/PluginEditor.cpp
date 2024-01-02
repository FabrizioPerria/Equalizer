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

    addAndMakeVisible (eqParamContainer);

    addAndMakeVisible (globalBypassButton);
    addAndMakeVisible (bypassButtonContainer);

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
    pluginBounds.removeFromTop (2 * pluginMargin);

    auto stereoMeterWidth = MONO_METER_WIDTH + METER_SCALE_WIDTH + MONO_METER_WIDTH;
    inputMeter.setBounds (pluginBounds.removeFromLeft (stereoMeterWidth));
    outputMeter.setBounds (pluginBounds.removeFromRight (stereoMeterWidth));

    pluginBounds.reduce (2 * pluginMargin, 0);

    auto buttonHeight = JUCE_LIVE_CONSTANT (20);
    auto globalBypassButtonBounds = pluginBounds.removeFromTop (buttonHeight);
    globalBypassButton.setBounds (globalBypassButtonBounds.removeFromRight (JUCE_LIVE_CONSTANT (40)));

    pluginBounds.removeFromTop (2 * pluginMargin);

    auto bypassButtonContainerBounds = pluginBounds.removeFromTop (buttonHeight);
    bypassButtonContainer.setBounds (bypassButtonContainerBounds);

    auto eqParamWidgetBounds = pluginBounds.removeFromBottom (EqParamContainer::sliderArea + EqParamContainer::buttonArea);
    eqParamContainer.setBounds (eqParamWidgetBounds);
}

void EqualizerAudioProcessorEditor::timerCallback()
{
    updateMeterValues (audioProcessor.inMeterValuesFifo, inputMeter);
    updateMeterValues (audioProcessor.outMeterValuesFifo, outputMeter);
}
