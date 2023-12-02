#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

using Filter = juce::dsp::IIR::Filter<float>;
using Coefficients = juce::dsp::IIR::Coefficients<float>::Ptr;
using MonoFilter = juce::dsp::ProcessorChain<Filter>;

static const int NUM_FILTERS = 1;

void updateFilters (juce::AudioProcessorValueTreeState& apvts, //
                    MonoFilter& leftChannel,
                    MonoFilter& rightChannel,
                    double sampleRate);
