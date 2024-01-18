/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "utils/AnalyzerProperties.h"
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
    addAndMakeVisible (controls);

    addAndMakeVisible (spectrumAnalyzer);
    addAndMakeVisible (responseCurve);

    audioProcessor.addSampleRateListener (this);

    startTimerHz (FRAMES_PER_SECOND);
}

EqualizerAudioProcessorEditor::~EqualizerAudioProcessorEditor()
{
    audioProcessor.removeSampleRateListener (this);
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
    auto pluginBounds = getLocalBounds().reduced (pluginMargin);
    auto analyzerControlsBounds = pluginBounds.removeFromBottom (100);
    controls.setBounds (analyzerControlsBounds);

    pluginBounds.removeFromLeft (pluginMargin);
    pluginBounds.removeFromRight (pluginMargin);
    pluginBounds.removeFromTop (2 * pluginMargin);
    pluginBounds.removeFromBottom (pluginMargin);

    auto stereoMeterWidth = MONO_METER_WIDTH + METER_SCALE_WIDTH + MONO_METER_WIDTH;
    inputMeter.setBounds (pluginBounds.removeFromLeft (stereoMeterWidth));
    outputMeter.setBounds (pluginBounds.removeFromRight (stereoMeterWidth));

    pluginBounds.reduce (2 * pluginMargin, 0);

    auto buttonHeight = 20;
    auto globalBypassButtonBounds = pluginBounds.removeFromTop (buttonHeight).reduced (2 * pluginMargin, 0);
    globalBypassButton.setBounds (globalBypassButtonBounds.removeFromRight (3 * buttonHeight));

    pluginBounds.removeFromTop (2 * pluginMargin);

    auto bypassButtonContainerBounds = pluginBounds.removeFromTop (buttonHeight);
    bypassButtonContainer.setBounds (bypassButtonContainerBounds);

    auto eqParamWidgetBounds = pluginBounds.removeFromBottom (EqParamContainer::sliderArea + EqParamContainer::buttonArea);
    eqParamContainer.setBounds (eqParamWidgetBounds);

    pluginBounds.reduce (0, pluginMargin);
    spectrumAnalyzer.setBounds (pluginBounds);
    responseCurve.setBounds(pluginBounds);
}

void EqualizerAudioProcessorEditor::timerCallback()
{
    updateMeterValues (audioProcessor.inMeterValuesFifo, inputMeter);
    updateMeterValues (audioProcessor.outMeterValuesFifo, outputMeter);

#if USE_TEST_SIGNAL
    int step_time = JUCE_LIVE_CONSTANT (60); // 60Hz * 60 = 1s
    if (counter < step_time)
    {
        counter++;
    }
    else
    {
        counter = 0;

        auto sampleRate = audioProcessor.getSampleRate();
        auto fftSize = 1 << static_cast<int> (audioProcessor.getCurrentFFTOrder());
        auto minBinNum = static_cast<size_t> (juce::roundToInt (20.f / sampleRate * fftSize));
        auto maxBinNum = static_cast<size_t> (juce::roundToInt (20000.f / sampleRate * fftSize));
        auto bin = audioProcessor.binNum.load();
#if MOVE_FORWARD_AND_WRAP
        if (++bin >= maxBinNum)
        {
            bin = minBinNum;
        }
#else
        if (bin >= maxBinNum)
        {
            step = -1;
        }

        if (bin <= minBinNum)
        {
            step = 1;
        }

        bin += step;
#endif
        audioProcessor.binNum.store (bin);
    }
#endif
}

void EqualizerAudioProcessorEditor::sampleRateChanged (double newSampleRate)
{
    spectrumAnalyzer.changeSampleRate (newSampleRate);
}
