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
    addAndMakeVisible (controls);

#ifdef PATH_PRODUCER_TEST
    pathProducer.setDecayRate (120.f);
    pathProducer.changeOrder (audioProcessor.fftOrder);
#else
    addAndMakeVisible (spectrumAnalyzer);
#endif

    audioProcessor.addSRListener (this);
    startTimerHz (FRAMES_PER_SECOND);
}

EqualizerAudioProcessorEditor::~EqualizerAudioProcessorEditor()
{
    audioProcessor.removeSRListener (this);
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

#ifdef PATH_PRODUCER_TEST
    g.setColour (juce::Colours::red);

    g.strokePath (currentPath, juce::PathStrokeType (1));
    g.drawRoundedRectangle (fftBounds, 4, 1);
#endif
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
#ifdef PATH_PRODUCER_TEST
    fftBounds = pluginBounds.toFloat();
    pathProducer.setFFTRectBounds (fftBounds);
#else
    spectrumAnalyzer.setBounds (pluginBounds);
#endif
}

void EqualizerAudioProcessorEditor::timerCallback()
{
    updateMeterValues (audioProcessor.inMeterValuesFifo, inputMeter);
    updateMeterValues (audioProcessor.outMeterValuesFifo, outputMeter);

#ifdef PATH_PRODUCER_TEST
    if (pathProducer.getNumAvailableForReading() > 0)
    {
        while (pathProducer.getNumAvailableForReading() > 0)
        {
            pathProducer.pull (currentPath);
        }
        repaint();
    }
#endif
}

void EqualizerAudioProcessorEditor::sampleRateChanged (double newSampleRate)
{
    spectrumAnalyzer.changeSampleRate (newSampleRate);
}
