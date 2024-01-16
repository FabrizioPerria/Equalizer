/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "ui/BypassButtonContainer.h"
#include "ui/ControlsComponent.h"
#include "ui/EqParamContainer.h"
#include "ui/GlobalBypassButton.h"
#include "ui/SpectrumAnalyzer.h"
#include "ui/StereoMeterComponent.h"
#include <JuceHeader.h>

//==============================================================================
/**
*/
class EqualizerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      public juce::Timer,
                                      public EqualizerAudioProcessor::SampleRateListener
{
public:
    EqualizerAudioProcessorEditor (EqualizerAudioProcessor&);
    ~EqualizerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

    void sampleRateChanged (double newSampleRate) override;

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

    EqParamContainer eqParamContainer { audioProcessor.apvts };

    GlobalBypassButton globalBypassButton { audioProcessor };
    BypassButtonContainer bypassButtonContainer { audioProcessor.apvts };
    ControlsComponent controls { audioProcessor.apvts };

    const int pluginMargin { 5 };

    SpectrumAnalyzer<juce::AudioBuffer<float>> spectrumAnalyzer { audioProcessor.getSampleRate(),
                                                                  audioProcessor.spectrumAnalyzerFifoLeft,
                                                                  audioProcessor.spectrumAnalyzerFifoRight,
                                                                  audioProcessor.apvts };

#if USE_TEST_SIGNAL
    int counter { 0 };
    int step { 1 };
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessorEditor)
};
