/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "data/FilterParameters.h"
#include "utils/FilterType.h"
#include "utils/CoefficientsMaker.h"
#include <JuceHeader.h>

//==============================================================================
/**
*/
class EqualizerAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    EqualizerAudioProcessor();
    ~EqualizerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Params", createParameterLayout() };

    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>::Ptr;
    using MonoFilter = juce::dsp::ProcessorChain<Filter>;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    static const std::map<FilterInfo::FilterType, juce::String> filterTypeMap;
    FilterInfo::FilterType getFilterType (int filterIndex);
    static juce::String getFilterTypeName (FilterInfo::FilterType filterType);
    static juce::StringArray getFilterTypeNames();

    FilterParametersBase getBaseParameters (int filterIndex);
    FilterParameters getParametricParameters (int filterIndex, FilterInfo::FilterType filterType);
    HighCutLowCutParameters getCutParameters (int filterIndex, FilterInfo::FilterType filterType);
    bool needsParametricParams (FilterInfo::FilterType type);

    void setBypassed (MonoFilter& filter, int filterIndex, bool bypassed);
    void updateCoefficients (MonoFilter& filter, int filterIndex, Coefficients coefficients);

    template <typename ParamsType>
    void updateFilter (int filterIndex, ParamsType& oldParams, const ParamsType& newParams)
    {
        if (newParams != oldParams)
        {
            setBypassed (leftChain, filterIndex, newParams.bypassed);
            setBypassed (rightChain, filterIndex, newParams.bypassed);
            auto coefficients = CoefficientsMaker<float>::make (newParams, getSampleRate());
            updateCoefficients (leftChain, filterIndex, coefficients);
            updateCoefficients (rightChain, filterIndex, coefficients);
            oldParams = newParams;
        }
    }

    void updateFilters();

    static const int NUM_FILTERS = 1;

    std::array<FilterParameters, NUM_FILTERS> oldFilterParams;
    std::array<HighCutLowCutParameters, NUM_FILTERS> oldHighCutLowCutParams;
    MonoFilter leftChain, rightChain;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessor)
};
