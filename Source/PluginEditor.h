/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "ui/EqParamWidget.h"
#include "ui/StereoMeterComponent.h"
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

    template <typename FifoType, typename MeterType>
    void updateMeterValues (FifoType& fifo, MeterType& meter)
    {
        if (fifo.getNumAvailableForReading() > 0)
        {
            MeterValues meterValues;
            while (fifo.getNumAvailableForReading() > 0)
            {
                auto success = fifo.pull (meterValues);
                jassert (success);
            }

            meter.update (meterValues);
        }
    }

    StereoMeterComponent inputMeter { "PRE EQ" };
    StereoMeterComponent outputMeter { "POST EQ" };

    EqParamWidget eqParamWidget0 { audioProcessor.apvts, 0, true };
    EqParamWidget eqParamWidget1 { audioProcessor.apvts, 1, false };
    EqParamWidget eqParamWidget2 { audioProcessor.apvts, 2, false };
    EqParamWidget eqParamWidget3 { audioProcessor.apvts, 3, false };
    EqParamWidget eqParamWidget4 { audioProcessor.apvts, 4, false };
    EqParamWidget eqParamWidget5 { audioProcessor.apvts, 5, false };
    EqParamWidget eqParamWidget6 { audioProcessor.apvts, 6, false };
    EqParamWidget eqParamWidget7 { audioProcessor.apvts, 7, true };

    const int pluginMargin { 5 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessorEditor)
};
