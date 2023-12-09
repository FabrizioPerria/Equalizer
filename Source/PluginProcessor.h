/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "data/FilterParameters.h"
#include "utils/CoefficientsMaker.h"
#include "utils/Fifo.h"
#include "utils/FilterType.h"
#include <JuceHeader.h>

//==============================================================================
enum ChainPositions
{
    LOWCUT,
    FILTER1,
    HIGHCUT,
    NUM_FILTERS
};

// ====================================================================================================
enum Slope
{
    SLOPE_6 = 1,
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
    using Coefficients = juce::dsp::IIR::Coefficients<float>::Ptr;
    using CutCoefficients = juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>>;
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter>;
    using MonoFilter = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    static const std::map<FilterInfo::FilterType, juce::String> filterTypeMap;
    FilterInfo::FilterType getFilterType (int filterIndex);
    static bool isCutFilter (FilterInfo::FilterType filterType);
    static juce::StringArray getFilterTypeNames();

    float getRawParameter (int filterIndex, FilterInfo::FilterParam filterParameter);

    FilterParametersBase getBaseParameters (int filterIndex);
    FilterParameters getParametricParameters (int filterIndex, FilterInfo::FilterType filterType);
    HighCutLowCutParameters getCutParameters (int filterIndex, FilterInfo::FilterType filterType);

    void updateCoefficients (Coefficients& oldCoefficients, const Coefficients& newCoefficients);

    template <int Index, typename ChainType>
    void update (ChainType& chain, CutCoefficients& coefficients)
    {
        updateCoefficients (chain.template get<Index>().coefficients, coefficients[Index]);
        chain.template setBypassed<Index> (false);
    }

    template <typename ChainType>
    void updateCutFilter (ChainType& chain, CutCoefficients& coefficients, Slope slope)
    {
        chain.template setBypassed<0> (true);
        chain.template setBypassed<1> (true);
        chain.template setBypassed<2> (true);
        chain.template setBypassed<3> (true);
        chain.template setBypassed<4> (true);
        chain.template setBypassed<5> (true);
        chain.template setBypassed<6> (true);
        chain.template setBypassed<7> (true);

        switch (slope)
        {
            case Slope::SLOPE_48:
            case Slope::SLOPE_40:
            {
                update<3> (chain, coefficients);
            }
            case Slope::SLOPE_36:
            case Slope::SLOPE_30:
            {
                update<2> (chain, coefficients);
            }
            case Slope::SLOPE_24:
            case Slope::SLOPE_18:
            {
                update<1> (chain, coefficients);
            }
            case Slope::SLOPE_12:
            case Slope::SLOPE_6:
            {
                update<0> (chain, coefficients);
            }
        }
    }

    template <int ChainPosition, typename ParamsType>
    void updateFilter (ParamsType& oldParams, const ParamsType& newParams)
    {
        if (newParams != oldParams)
        {
            leftChain.setBypassed<ChainPosition> (newParams.bypassed);
            rightChain.setBypassed<ChainPosition> (newParams.bypassed);

            auto coefficients = CoefficientsMaker<float>::make (newParams, getSampleRate());
            auto& leftFilter = leftChain.template get<ChainPosition>();
            auto& rightFilter = rightChain.template get<ChainPosition>();

            if constexpr (ChainPosition == HIGHCUT)
            {
                highcutFilterFifo.push (coefficients);
                CutCoefficients pulledCoefficients;
                highcutFilterFifo.pull (pulledCoefficients);
                updateCutFilter (leftFilter, pulledCoefficients, static_cast<Slope> (newParams.order));
                updateCutFilter (rightFilter, pulledCoefficients, static_cast<Slope> (newParams.order));
            }
            else if constexpr (ChainPosition == LOWCUT)
            {
                lowcutFilterFifo.push (coefficients);
                CutCoefficients pulledCoefficients;
                lowcutFilterFifo.pull (pulledCoefficients);
                updateCutFilter (leftFilter, pulledCoefficients, static_cast<Slope> (newParams.order));
                updateCutFilter (rightFilter, pulledCoefficients, static_cast<Slope> (newParams.order));
            }
            else
            {
                parametricFilterFifo.push (coefficients);
                Coefficients pulledCoefficients;
                parametricFilterFifo.pull (pulledCoefficients);
                updateCoefficients (leftFilter.coefficients, pulledCoefficients);
                updateCoefficients (rightFilter.coefficients, pulledCoefficients);
            }
            oldParams = newParams;
        }
    }

    void updateFilters();

    std::array<FilterParameters, ChainPositions::NUM_FILTERS> oldFilterParams;
    std::array<HighCutLowCutParameters, ChainPositions::NUM_FILTERS> oldHighCutLowCutParams;

    MonoFilter leftChain, rightChain;

    Fifo<CutCoefficients, 100> lowcutFilterFifo;
    Fifo<Coefficients, 100> parametricFilterFifo;
    Fifo<CutCoefficients, 100> highcutFilterFifo;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessor)
};
