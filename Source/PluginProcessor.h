/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "utils/FFTDataGenerator.h"
#define USE_TEST_OSC true

#include "data/FilterLink.h"
#include "data/FilterParameters.h"
#include "data/MeterValues.h"
#include "utils/CoefficientsMaker.h"
#include "utils/EqParam.h"
#include "utils/FilterParam.h"
#include "utils/FilterType.h"
#include "utils/MidSideProcessor.h"
#include "utils/SingleChannelSampleFifo.h"
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
    HIGHCUT
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

    void setGlobalBypass (bool bypass);
    bool isAnyFilterActive();

    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Params", createParameterLayout() };

    Fifo<MeterValues, 20> inMeterValuesFifo;
    Fifo<MeterValues, 20> outMeterValuesFifo;

    SingleChannelSampleFifo<juce::AudioBuffer<float>> spectrumAnalyzerFifoLeft { Channel::LEFT };
    SingleChannelSampleFifo<juce::AudioBuffer<float>> spectrumAnalyzerFifoRight { Channel::RIGHT };

    FFTOrder fftOrder { FFTOrder::order2048 };

    using GainTrim = juce::dsp::Gain<float>;
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
        for (auto audioChannel : { Channel::LEFT, Channel::RIGHT })
        {
            auto index = static_cast<int> (FilterPosition);
            auto name = FilterInfo::getParameterName (index, audioChannel, FilterInfo::FilterParam::BYPASS);
            layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { name, 1 }, //
                                                                    name,
                                                                    false));

            name = FilterInfo::getParameterName (index, audioChannel, FilterInfo::FilterParam::FREQUENCY);
            auto range = juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.25f);
            layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { name, 1 }, //
                                                                     name,
                                                                     range,
                                                                     20.0f));

            name = FilterInfo::getParameterName (index, audioChannel, FilterInfo::FilterParam::Q);
            layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { name, 1 },
                                                                     name,
                                                                     juce::NormalisableRange<float> (0.1f, 10.0f, 0.01f),
                                                                     0.71f));
            if (isCutFilter)
            {
                name = FilterInfo::getParameterName (index, audioChannel, FilterInfo::FilterParam::SLOPE);
                layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { name, 1 }, //
                                                                          name,
                                                                          getSlopeNames(),
                                                                          0));
            }
            else
            {
                name = FilterInfo::getParameterName (index, audioChannel, FilterInfo::FilterParam::GAIN);
                layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { name, 1 },
                                                                         name,
                                                                         juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f),
                                                                         0.0f));
            }
        }
    }

    static void addEqModeParameterToLayout (juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    static void addGainTrimParameterToLayout (juce::AudioProcessorValueTreeState::ParameterLayout& layout, const juce::String& name);

    static juce::StringArray getSlopeNames();

    float getRawParameter (const juce::String& name);
    float getRawFilterParameter (int filterIndex, Channel audioChannel, FilterInfo::FilterParam filterParameter);
    FilterParametersBase getBaseParameters (int filterIndex, Channel audioChannel);
    FilterParameters getParametricParameters (int filterIndex, Channel audioChannel, FilterInfo::FilterType filterType);
    HighCutLowCutParameters getCutParameters (int filterIndex, Channel audioChannel, FilterInfo::FilterType filterType);

    EqMode getEqMode();

    void initializeFilters();

    template <ChainPositions FilterPosition>
    void initializeCutFilter (FilterInfo::FilterType filterType, EqMode mode, bool onRealTimeThread)
    {
        const int filterIndex = static_cast<int> (FilterPosition);
        auto leftCutParams = getCutParameters (filterIndex, Channel::LEFT, filterType);
        leftChain.get<filterIndex>().initialize (leftCutParams, RAMP_TIME_IN_SECONDS, onRealTimeThread, getSampleRate());

        auto rightCutParams = mode == EqMode::STEREO ? leftCutParams //
                                                     : getCutParameters (filterIndex, Channel::RIGHT, filterType);
        rightChain.get<filterIndex>().initialize (rightCutParams, RAMP_TIME_IN_SECONDS, onRealTimeThread, getSampleRate());
    }

    template <ChainPositions FilterPosition>
    void initializeParametricFilter (FilterInfo::FilterType filterType, EqMode mode, bool onRealTimeThread)
    {
        const int filterIndex = static_cast<int> (FilterPosition);
        auto leftParametricParams = getParametricParameters (filterIndex, Channel::LEFT, filterType);
        leftChain.get<filterIndex>().initialize (leftParametricParams, RAMP_TIME_IN_SECONDS, onRealTimeThread, getSampleRate());

        auto rightParametricParams = mode == EqMode::STEREO ? leftParametricParams
                                                            : getParametricParameters (filterIndex, Channel::RIGHT, filterType);
        rightChain.get<filterIndex>().initialize (rightParametricParams, RAMP_TIME_IN_SECONDS, onRealTimeThread, getSampleRate());
    }

    void updateParameters (EqMode mode);

    template <ChainPositions FilterPosition>
    void updateCutParameters (FilterInfo::FilterType filterType, EqMode mode)
    {
        const int filterIndex = static_cast<int> (FilterPosition);
        auto leftCutParams = getCutParameters (filterIndex, Channel::LEFT, filterType);
        leftChain.get<filterIndex>().performPreloopUpdate (leftCutParams);

        auto rightCutParams = mode == EqMode::STEREO ? leftCutParams //
                                                     : getCutParameters (filterIndex, Channel::RIGHT, filterType);
        rightChain.get<filterIndex>().performPreloopUpdate (rightCutParams);
    }

    template <ChainPositions FilterPosition>
    void updateParametricParameters (FilterInfo::FilterType filterType, EqMode mode)
    {
        const int filterIndex = static_cast<int> (FilterPosition);
        auto leftParametricParams = getParametricParameters (filterIndex, Channel::LEFT, filterType);
        leftChain.get<filterIndex>().performPreloopUpdate (leftParametricParams);

        auto rightParametricParams = mode == EqMode::STEREO ? leftParametricParams //
                                                            : getParametricParameters (filterIndex, Channel::RIGHT, filterType);
        rightChain.get<filterIndex>().performPreloopUpdate (rightParametricParams);
    }

    void updateFilters (int chunkSize);

    template <ChainPositions FilterPosition>
    void updateFilter (bool onRealTimeThread, int chunkSize)
    {
        const int filterIndex = static_cast<int> (FilterPosition);
        leftChain.get<filterIndex>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);
        rightChain.get<filterIndex>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);
    }

    template <typename FifoType, typename BufferType>
    void updateMeterFifos (FifoType& fifo, BufferType& buffer)
    {
        const auto leftChannel = static_cast<int> (Channel::LEFT);
        const auto rightChannel = static_cast<int> (Channel::RIGHT);
        MeterValues meterValues;
        meterValues.leftPeakDb.setGain (buffer.getMagnitude (leftChannel, 0, buffer.getNumSamples()));
        meterValues.rightPeakDb.setGain (buffer.getMagnitude (rightChannel, 0, buffer.getNumSamples()));
        meterValues.leftRmsDb.setGain (buffer.getRMSLevel (leftChannel, 0, buffer.getNumSamples()));
        meterValues.rightRmsDb.setGain (buffer.getRMSLevel (rightChannel, 0, buffer.getNumSamples()));

        fifo.push (meterValues);
    }

    template <ChainPositions FilterPosition>
    void setupBypassFilter (bool bypass)
    {
        setupBypassMonoFilter<FilterPosition, Channel::LEFT> (bypass);
        setupBypassMonoFilter<FilterPosition, Channel::RIGHT> (bypass);
    }

    template <ChainPositions FilterPosition, Channel FilterChannel>
    void setupBypassMonoFilter (bool bypass)
    {
        auto bypassName = FilterInfo::getParameterName (static_cast<int> (FilterPosition), FilterChannel, FilterInfo::FilterParam::BYPASS);
        auto param = dynamic_cast<juce::AudioParameterBool*> (apvts.getParameter (bypassName));
        param->beginChangeGesture();
        *param = bypass;
        param->endChangeGesture();
    }

    template <ChainPositions FilterPosition>
    bool isFilterActive()
    {
        bool isActive = isMonoFilterActive<FilterPosition, Channel::LEFT>();
        if (getEqMode() != EqMode::STEREO)
        {
            isActive |= isMonoFilterActive<FilterPosition, Channel::RIGHT>();
        }
        return isActive;
    }

    template <ChainPositions FilterPosition, Channel FilterChannel>
    bool isMonoFilterActive()
    {
        auto bypassName = FilterInfo::getParameterName (static_cast<int> (FilterPosition), FilterChannel, FilterInfo::FilterParam::BYPASS);
        return getRawParameter (bypassName) < 0.5f;
    }

    void updateTrimGains();

    MonoChain leftChain, rightChain;
    GainTrim inputGain, outputGain;

    MidSideProcessor midSideProcessor;

#ifdef USE_TEST_OSC
    juce::dsp::Gain<float> testGain;
    juce::dsp::Oscillator<float> testOscillator { [] (float x) { return std::sin (x); } };
#endif

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessor)
};
