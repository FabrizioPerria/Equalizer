#pragma once

#include "ui/AnalyzerBase.h"
#include "PluginProcessor.h"
#include "utils/AllParamsListener.h"
#include "utils/ChainHelpers.h"
#include <JuceHeader.h>

struct ResponseCurveComponent : AnalyzerBase
{
    ResponseCurveComponent (EqualizerAudioProcessor& p);
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    EqualizerAudioProcessor& audioProcessor;

    juce::AudioProcessorValueTreeState* apvts = {&audioProcessor.apvts};
    double sampleRate {audioProcessor.getSampleRate()};
    std::unique_ptr<AllParamsListener> allParamsListener;

    ChainHelpers::MonoChain leftChain, rightChain;
    juce::Path leftResponseCurve, rightResponseCurve;

    void refreshParams();
    void buildNewResponseCurves();
    void updateChainParameters();
    void buildNewResponseCurve (std::vector<float>& path, ChainHelpers::MonoChain& chain);
    void createResponseCurve (juce::Path& path, const std::vector<float>& data);
};
