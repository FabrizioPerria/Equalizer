/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "data/FilterLink.h"
#include "data/FilterParameters.h"
#include "utils/CoefficientsMaker.h"
#include "utils/FilterParam.h"
#include "utils/FilterType.h"
#include <JuceHeader.h>

//==============================================================================
enum class Channel
{
    LEFT,
    RIGHT
};
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

    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    using CoefficientsPtr = Coefficients::Ptr;
    using CutCoefficients = juce::ReferenceCountedArray<Coefficients>;
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

    using CutFilterLink = FilterLink<CutFilter, CutCoefficients, HighCutLowCutParameters, CoefficientsMaker<float>>;
    using SingleFilterLink = FilterLink<Filter, CoefficientsPtr, FilterParameters, CoefficientsMaker<float>>;

    using MonoChain = juce::dsp::ProcessorChain<CutFilterLink,    //lowCut
                                                SingleFilterLink, //lowShelf
                                                SingleFilterLink, //Peak1
                                                SingleFilterLink, //Peak2
                                                SingleFilterLink, //Peak3
                                                SingleFilterLink, //Peak4
                                                SingleFilterLink, //HighShelf
                                                CutFilterLink>;   //HighCut

private:
    const float RAMP_TIME_IN_SECONDS = 0.05f;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    template <ChainPositions FilterPosition>
    static void addFilterParameterToLayout (juce::AudioProcessorValueTreeState::ParameterLayout& layout, bool isCutFilter)
    {
        auto index = static_cast<int> (FilterPosition);
        auto name = FilterInfo::getParameterName (index, FilterInfo::FilterParam::BYPASS);
        layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { name, 1 }, //
                                                                name,
                                                                false));

        name = FilterInfo::getParameterName (index, FilterInfo::FilterParam::FREQUENCY);
        auto range = juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.25f);
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { name, 1 }, //
                                                                 name,
                                                                 range,
                                                                 20.0f));

        name = FilterInfo::getParameterName (index, FilterInfo::FilterParam::Q);
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { name, 1 },
                                                                 name,
                                                                 juce::NormalisableRange<float> (0.1f, 10.0f, 0.1f),
                                                                 1.0f));
        if (isCutFilter)
        {
            name = FilterInfo::getParameterName (index, FilterInfo::FilterParam::SLOPE);
            layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { name, 1 }, //
                                                                      name,
                                                                      getSlopeNames(),
                                                                      0));
        }
        else
        {
            name = FilterInfo::getParameterName (index, FilterInfo::FilterParam::GAIN);
            layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { name, 1 },
                                                                     name,
                                                                     juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f),
                                                                     0.0f));
        }
    }

    static juce::StringArray getSlopeNames();

    float getRawParameter (int filterIndex, FilterInfo::FilterParam filterParameter);

    FilterParametersBase getBaseParameters (int filterIndex);
    FilterParameters getParametricParameters (int filterIndex, FilterInfo::FilterType filterType);
    HighCutLowCutParameters getCutParameters (int filterIndex, FilterInfo::FilterType filterType);

    void initializeFilters();

    template <ChainPositions FilterPosition>
    void initializeCutFilter (FilterInfo::FilterType filterType, bool onRealTimeThread)
    {
        const int filterIndex = static_cast<int> (FilterPosition);
        auto cutParams = getCutParameters (filterIndex, filterType);
        leftChain.get<filterIndex>().initialize (cutParams, RAMP_TIME_IN_SECONDS, onRealTimeThread, getSampleRate());
        rightChain.get<filterIndex>().initialize (cutParams, RAMP_TIME_IN_SECONDS, onRealTimeThread, getSampleRate());
    }

    template <ChainPositions FilterPosition>
    void initializeParametricFilter (FilterInfo::FilterType filterType, bool onRealTimeThread)
    {
        const int filterIndex = static_cast<int> (FilterPosition);
        auto parametricParams = getParametricParameters (filterIndex, filterType);
        leftChain.get<filterIndex>().initialize (parametricParams, RAMP_TIME_IN_SECONDS, onRealTimeThread, getSampleRate());
        rightChain.get<filterIndex>().initialize (parametricParams, RAMP_TIME_IN_SECONDS, onRealTimeThread, getSampleRate());
    }

    void updateParameters();

    template <ChainPositions FilterPosition>
    void updateCutParameters (FilterInfo::FilterType filterType)
    {
        const int filterIndex = static_cast<int> (FilterPosition);
        auto cutParams = getCutParameters (filterIndex, filterType);
        leftChain.get<filterIndex>().performPreloopUpdate (cutParams);
        rightChain.get<filterIndex>().performPreloopUpdate (cutParams);
    }

    template <ChainPositions FilterPosition>
    void updateParametricParameters (FilterInfo::FilterType filterType)
    {
        const int filterIndex = static_cast<int> (FilterPosition);
        auto parametricParams = getParametricParameters (filterIndex, filterType);
        leftChain.get<filterIndex>().performPreloopUpdate (parametricParams);
        rightChain.get<filterIndex>().performPreloopUpdate (parametricParams);
    }

    void updateFilters (int chunkSize);

    template <ChainPositions FilterPosition>
    void updateFilter (bool onRealTimeThread, int chunkSize)
    {
        const int filterIndex = static_cast<int> (FilterPosition);
        leftChain.get<filterIndex>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);
        rightChain.get<filterIndex>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);
    }

    MonoChain leftChain, rightChain;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessor)
};
