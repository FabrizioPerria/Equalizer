#pragma once

#include "data/FilterLink.h"
#include "data/FilterParameters.h"
#include "utils/CoefficientsMaker.h"
#include "utils/EqParam.h"
#include "utils/FilterParam.h"
#include "utils/FilterType.h"
#include <JuceHeader.h>

#define RAMP_TIME_IN_SECONDS 0.05f

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

namespace ChainHelpers
{
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

template <int FilterIndex>
float getRawFilterParameter (Channel audioChannel, FilterInfo::FilterParam filterParameter, juce::AudioProcessorValueTreeState& apvts)
{
    auto name = FilterInfo::getParameterName (FilterIndex, audioChannel, filterParameter);
    return apvts.getRawParameterValue (name)->load();
}

template <int FilterIndex>
FilterParametersBase getBaseParameters (Channel audioChannel, double sampleRate, juce::AudioProcessorValueTreeState& apvts)
{
    auto frequencyParam = getRawFilterParameter<FilterIndex> (audioChannel, FilterInfo::FilterParam::FREQUENCY, apvts);
    auto bypassParamRaw = getRawFilterParameter<FilterIndex> (audioChannel, FilterInfo::FilterParam::BYPASS, apvts);
    auto bypassParam = bypassParamRaw > 0.5f;
    auto qParam = getRawFilterParameter<FilterIndex> (audioChannel, FilterInfo::FilterParam::Q, apvts);

    return FilterParametersBase { frequencyParam, bypassParam, qParam, sampleRate };
}

template <int FilterIndex>
FilterParameters getParametricParameters (Channel audioChannel,
                                          FilterInfo::FilterType filterType,
                                          double sampleRate,
                                          juce::AudioProcessorValueTreeState& apvts)
{
    auto baseParams = getBaseParameters<FilterIndex> (audioChannel, sampleRate, apvts);
    auto gainParam = Decibel<float> (getRawFilterParameter<FilterIndex> (audioChannel, FilterInfo::FilterParam::GAIN, apvts));

    return FilterParameters { baseParams, filterType, gainParam };
}

template <int FilterIndex>
HighCutLowCutParameters
    getCutParameters (Channel audioChannel, FilterInfo::FilterType filterType, double sampleRate, juce::AudioProcessorValueTreeState& apvts)
{
    auto baseParams = getBaseParameters<FilterIndex> (audioChannel, sampleRate, apvts);
    auto isLowCutParam = filterType == FilterInfo::FilterType::HIGHPASS;
    auto slopeParam = static_cast<int> (getRawFilterParameter<FilterIndex> (audioChannel, FilterInfo::FilterParam::SLOPE, apvts));

    return HighCutLowCutParameters { baseParams, slopeParam, isLowCutParam };
}

template <ChainPositions FilterPosition>
void initializeCutFilter (MonoChain& leftChain,
                          MonoChain& rightChain,
                          FilterInfo::FilterType filterType,
                          EqMode mode,
                          bool onRealTimeThread,
                          double sampleRate,
                          juce::AudioProcessorValueTreeState& apvts)
{
    const int FilterIndex = static_cast<int> (FilterPosition);
    auto leftCutParams = getCutParameters<FilterIndex> (Channel::LEFT, filterType, sampleRate, apvts);
    leftChain.get<FilterIndex>().initialize (leftCutParams, RAMP_TIME_IN_SECONDS, onRealTimeThread, sampleRate);

    auto rightCutParams = mode == EqMode::STEREO ? leftCutParams //
                                                 : getCutParameters<FilterIndex> (Channel::RIGHT, filterType, sampleRate, apvts);
    rightChain.get<FilterIndex>().initialize (rightCutParams, RAMP_TIME_IN_SECONDS, onRealTimeThread, sampleRate);
}

template <ChainPositions FilterPosition>
void initializeParametricFilter (MonoChain& leftChain,
                                 MonoChain& rightChain,
                                 FilterInfo::FilterType filterType,
                                 EqMode mode,
                                 bool onRealTimeThread,
                                 double sampleRate,
                                 juce::AudioProcessorValueTreeState& apvts)
{
    const int FilterIndex = static_cast<int> (FilterPosition);
    auto leftParametricParams = getParametricParameters<FilterIndex> (Channel::LEFT, filterType, sampleRate, apvts);
    leftChain.get<FilterIndex>().initialize (leftParametricParams, RAMP_TIME_IN_SECONDS, onRealTimeThread, sampleRate);

    auto rightParametricParams = mode == EqMode::STEREO
                                     ? leftParametricParams
                                     : getParametricParameters<FilterIndex> (Channel::RIGHT, filterType, sampleRate, apvts);
    rightChain.get<FilterIndex>().initialize (rightParametricParams, RAMP_TIME_IN_SECONDS, onRealTimeThread, sampleRate);
}

inline void initializeChains (MonoChain& leftChain, MonoChain& rightChain, double sampleRate, juce::AudioProcessorValueTreeState& apvts)
{
    bool onRealTimeThread = ! juce::MessageManager::getInstanceWithoutCreating()->isThisTheMessageThread();
    auto mode = static_cast<EqMode> (apvts.getRawParameterValue ("eq_mode")->load());
    initializeCutFilter<ChainPositions::LOWCUT> (leftChain,
                                                 rightChain,
                                                 FilterInfo::FilterType::HIGHPASS,
                                                 mode,
                                                 onRealTimeThread,
                                                 sampleRate,
                                                 apvts);
    initializeParametricFilter<ChainPositions::LOWSHELF> (leftChain,
                                                          rightChain,
                                                          FilterInfo::FilterType::LOWSHELF,
                                                          mode,
                                                          onRealTimeThread,
                                                          sampleRate,
                                                          apvts);
    initializeParametricFilter<ChainPositions::PEAK1> (leftChain,
                                                       rightChain,
                                                       FilterInfo::FilterType::PEAKFILTER,
                                                       mode,
                                                       onRealTimeThread,
                                                       sampleRate,
                                                       apvts);
    initializeParametricFilter<ChainPositions::PEAK2> (leftChain,
                                                       rightChain,
                                                       FilterInfo::FilterType::PEAKFILTER,
                                                       mode,
                                                       onRealTimeThread,
                                                       sampleRate,
                                                       apvts);
    initializeParametricFilter<ChainPositions::PEAK3> (leftChain,
                                                       rightChain,
                                                       FilterInfo::FilterType::PEAKFILTER,
                                                       mode,
                                                       onRealTimeThread,
                                                       sampleRate,
                                                       apvts);
    initializeParametricFilter<ChainPositions::PEAK4> (leftChain,
                                                       rightChain,
                                                       FilterInfo::FilterType::PEAKFILTER,
                                                       mode,
                                                       onRealTimeThread,
                                                       sampleRate,
                                                       apvts);
    initializeParametricFilter<ChainPositions::HIGHSHELF> (leftChain,
                                                           rightChain,
                                                           FilterInfo::FilterType::HIGHSHELF,
                                                           mode,
                                                           onRealTimeThread,
                                                           sampleRate,
                                                           apvts);
    initializeCutFilter<ChainPositions::HIGHCUT> (leftChain,
                                                  rightChain,
                                                  FilterInfo::FilterType::LOWPASS,
                                                  mode,
                                                  onRealTimeThread,
                                                  sampleRate,
                                                  apvts);

    leftChain.reset();
    rightChain.reset();
}

} // namespace ChainHelpers
