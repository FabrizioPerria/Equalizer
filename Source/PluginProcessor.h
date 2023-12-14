/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "data/FilterLink.h"
#include "data/FilterParameters.h"
#include "utils/CoefficientsMaker.h"
#include "utils/FilterType.h"
#include <JuceHeader.h>

//==============================================================================
enum class ChainPositions
{
    LOWCUT,
    LOWSHELF,
    PEAK1,
    PEAK2,
    PEAK3,
    PEAK4,
    HIGHSHELF,
    HIGHCUT,
    NUM_FILTERS
};

// ====================================================================================================
enum class Slope
{
    SLOPE_6,
    SLOPE_12,
    SLOPE_18,
    SLOPE_24,
    SLOPE_30,
    SLOPE_36,
    SLOPE_40,
    SLOPE_48
};
// ====================================================================================================
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

    using CutFilter = FilterLink<CutFilter, CutCoefficients, HighCutLowCutParameters, CoefficientsMaker<float>>;
    using SingleFilter = FilterLink<Filter, CoefficientsPtr, FilterParameters, CoefficientsMaker<float>>;

    using MonoFilter = juce::dsp::ProcessorChain<CutFilter,    //lowCut
                                                 SingleFilter, //lowShelf
                                                 SingleFilter, //Peak1
                                                 SingleFilter, //Peak2
                                                 SingleFilter, //Peak3
                                                 SingleFilter, //Peak4
                                                 SingleFilter, //HighShelf
                                                 CutFilter>;   //HighCut

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    static const std::map<FilterInfo::FilterType, juce::String> filterTypeMap;
    FilterInfo::FilterType getFilterType (int filterIndex);
    static juce::StringArray getFilterTypeNames();
    static juce::StringArray getSlopeNames();

    float getRawParameter (int filterIndex, FilterInfo::FilterParam filterParameter);

    FilterParametersBase getBaseParameters (int filterIndex);
    FilterParameters getParametricParameters (int filterIndex, FilterInfo::FilterType filterType);
    HighCutLowCutParameters getCutParameters (int filterIndex, FilterInfo::FilterType filterType);

    void updateFilters();

    MonoFilter leftChain, rightChain;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessor)
};
