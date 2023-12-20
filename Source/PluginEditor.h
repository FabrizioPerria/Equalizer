/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "ui/MeterComponent.h"
#include <JuceHeader.h>

//==============================================================================
/**
*/
class EqualizerAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    EqualizerAudioProcessorEditor (EqualizerAudioProcessor&);
    ~EqualizerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    EqualizerAudioProcessor& audioProcessor;

    MeterComponent inputMeter;
    juce::AudioBuffer<float> inputMeterBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessorEditor)
};
