/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "utils/EqParam.h"
//==============================================================================
EqualizerAudioProcessorEditor::EqualizerAudioProcessorEditor (EqualizerAudioProcessor& p) : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    addAndMakeVisible (inputMeter);
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
    auto editorWidth = getLocalBounds().getWidth();
    inputMeter.setBounds (getLocalBounds().reduced (5).withWidth (editorWidth / 30));
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
