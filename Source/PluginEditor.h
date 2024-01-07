/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "ui/AnalyzerControls.h"
#include "ui/BypassButtonContainer.h"
#include "ui/EqParamContainer.h"
#include "ui/GlobalBypassButton.h"
#include "ui/SpectrumAnalyzer.h"
#include "ui/StereoMeterComponent.h"
#include "utils/PathProducer.h"
#include <JuceHeader.h>

#define TEST_EQ_MODE true
/* #define PATH_PRODUCER_TEST true */

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

    EqParamContainer eqParamContainer { audioProcessor.apvts };

    GlobalBypassButton globalBypassButton { audioProcessor };
    BypassButtonContainer bypassButtonContainer { audioProcessor.apvts };
    AnalyzerControls analyzerControls { audioProcessor.apvts };

#ifdef TEST_EQ_MODE
    juce::ComboBox eqModeComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> eqModeComboBoxAttachment;
#endif
    const int pluginMargin { 5 };

#ifdef PATH_PRODUCER_TEST
    PathProducer<juce::AudioBuffer<float>> pathProducer { audioProcessor.getSampleRate(), audioProcessor.spectrumAnalyzerFifoLeft };
    juce::Rectangle<float> fftBounds;
    juce::Path currentPath;
#else
    SpectrumAnalyzer<juce::AudioBuffer<float>> spectrumAnalyzer { audioProcessor.getSampleRate(),
                                                                  audioProcessor.spectrumAnalyzerFifoLeft,
                                                                  audioProcessor.spectrumAnalyzerFifoRight,
                                                                  audioProcessor.apvts };
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessorEditor)
};
