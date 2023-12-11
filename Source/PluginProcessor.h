/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "data/FilterParameters.h"
#include "utils/CoefficientsMaker.h"
#include "utils/Fifo.h"
#include "utils/FilterCoefficientGenerator.h"
#include "utils/FilterType.h"
#include <JuceHeader.h>

//==============================================================================
enum class ChainPositions
{
    LOWCUT,
    PARAMETRIC_FILTER,
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
    using Coefficients = juce::dsp::IIR::Coefficients<float>::Ptr;
    using CutCoefficients = juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>>;
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    using MonoFilter = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

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

    void updateCoefficients (Coefficients& oldCoefficients, const Coefficients& newCoefficients);

    template <int Index, typename ChainType>
    void update (ChainType& chain, CutCoefficients& coefficients)
    {
        updateCoefficients (chain.template get<Index>().coefficients, coefficients[Index]);
        chain.template setBypassed<Index> (false);
    }

    template <typename ChainType>
    void updateCutFilter (ChainType& chain, CutCoefficients& coefficients)
    {
        chain.template setBypassed<0> (true);
        chain.template setBypassed<1> (true);
        chain.template setBypassed<2> (true);
        chain.template setBypassed<3> (true);

        switch (coefficients.size())
        {
            case 4:
            {
                update<3> (chain, coefficients);
            }
            case 3:
            {
                update<2> (chain, coefficients);
            }
            case 2:
            {
                update<1> (chain, coefficients);
            }
            case 1:
            {
                update<0> (chain, coefficients);
            }
        }
    }

    template <ChainPositions ChainPosition, typename ParamsType, typename CoefficientsGenerator>
    void updateFilter (ParamsType& oldParams, const ParamsType& newParams, CoefficientsGenerator& generator)
    {
        const int ChainPositionInt = static_cast<int> (ChainPosition);
        leftChain.setBypassed<ChainPositionInt> (newParams.bypassed);
        rightChain.setBypassed<ChainPositionInt> (newParams.bypassed);
        auto& leftFilter = leftChain.template get<ChainPositionInt>();
        auto& rightFilter = rightChain.template get<ChainPositionInt>();

        if (newParams != oldParams)
        {
            generator.changeParameters (newParams);
            oldParams = newParams;
        }

        const bool isHighCutFilter = ChainPosition == ChainPositions::HIGHCUT;
        const bool isLowCutFilter = ChainPosition == ChainPositions::LOWCUT;
        if constexpr (isHighCutFilter || isLowCutFilter)
        {
            CutCoefficients coefficients;

            auto& fifoToUse = isHighCutFilter ? highcutFilterFifo : lowcutFilterFifo;
            if (fifoToUse.pull (coefficients))
            {
                updateCutFilter (leftFilter, coefficients);
                updateCutFilter (rightFilter, coefficients);
            }
        }
        else
        {
            Coefficients coefficients;

            if (parametricFilterFifo.pull (coefficients))
            {
                updateCoefficients (leftFilter.coefficients, coefficients);
                updateCoefficients (rightFilter.coefficients, coefficients);

                // trying to prevent the object from being deleted. This means the object is never released, even if it's overwritten in the Fifo
                // TODO: make sure the object is released at some point, but outside the audio thread
                coefficients.get()->incReferenceCount();
            }
        }
    }

    void updateFilters();

    std::array<FilterParameters, static_cast<size_t> (ChainPositions::NUM_FILTERS)> oldFilterParams;
    std::array<HighCutLowCutParameters, static_cast<size_t> (ChainPositions::NUM_FILTERS)> oldHighCutLowCutParams;

    MonoFilter leftChain, rightChain;

    const static size_t FIFO_SIZE = 20;
    Fifo<CutCoefficients, FIFO_SIZE> lowcutFilterFifo;
    Fifo<Coefficients, FIFO_SIZE> parametricFilterFifo;
    Fifo<CutCoefficients, FIFO_SIZE> highcutFilterFifo;

    using CutCoefficientGenerator = FilterCoefficientGenerator<CutCoefficients,
                                                               HighCutLowCutParameters,
                                                               CoefficientsMaker<float>,
                                                               FIFO_SIZE>;

    CutCoefficientGenerator lowCutCoefficientsGenerator { lowcutFilterFifo };
    CutCoefficientGenerator highCutCoefficientsGenerator { highcutFilterFifo };

    using ParametricCoefficientGenerator = FilterCoefficientGenerator<Coefficients, //
                                                                      FilterParameters,
                                                                      CoefficientsMaker<float>,
                                                                      FIFO_SIZE>;

    ParametricCoefficientGenerator parametricCoefficientsGenerator { parametricFilterFifo };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessor)
};
