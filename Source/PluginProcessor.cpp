/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "utils/EqParam.h"
#include "utils/FilterParam.h"
#include "utils/FilterType.h"

//==============================================================================
EqualizerAudioProcessor::EqualizerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
#endif
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
}

EqualizerAudioProcessor::~EqualizerAudioProcessor()
{
}

//==============================================================================
const juce::String EqualizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EqualizerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool EqualizerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool EqualizerAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double EqualizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EqualizerAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int EqualizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EqualizerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EqualizerAudioProcessor::getProgramName (int index)
{
    return {};
}

void EqualizerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EqualizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (uint32_t) samplesPerBlock;
    spec.numChannels = 1;

    leftChain.prepare (spec);
    rightChain.prepare (spec);

    spec.numChannels = 2;
    inputGain.prepare (spec);
    outputGain.prepare (spec);

    inputMeterFifo.prepare (samplesPerBlock, 2);

    initializeFilters();

#ifdef USE_TEST_OSC
    testOscillator.prepare (spec);
    testOscillator.setFrequency (440.0f);
    testGain.prepare (spec);
#endif
}

void EqualizerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EqualizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    //if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
    //    && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    // NOTE: For some reason, the above code does not work with the AU plugin in Logic Pro and validation crashes.
    // Somehow, it expects the plugin to be only stereo. So, we will only support stereo for now.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void EqualizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto block = juce::dsp::AudioBlock<float> (buffer);

    updateTrimGains();

    auto mode = getEqMode();
    updateParameters (mode);

    inputGain.process (juce::dsp::ProcessContextReplacing<float> (block));

#ifdef USE_TEST_OSC
    testGain.setGainDecibels (JUCE_LIVE_CONSTANT (0.0f));
    for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
    {
        buffer.clear (channel, 0, buffer.getNumSamples());
        for (auto samplePosition = 0; samplePosition < buffer.getNumSamples(); ++samplePosition)
        {
            auto sample = osc.processSample (0.0f);
            buffer.setSample (channel, samplePosition, sample);
        }
    }
    testGain.process (juce::dsp::ProcessContextReplacing<float> (block));
#endif

    inputMeterFifo.push (buffer);

    if (mode == EqMode::MID_SIDE)
    {
        midSideProcessor.process (juce::dsp::ProcessContextReplacing<float> (block));
    }

    const size_t SUB_BLOCK_MAX_SIZE = 32;
    for (size_t offset = 0; offset < block.getNumSamples();)
    {
        auto numSamplesLeft = block.getNumSamples() - offset;
        auto maxChunkSize = juce::jmin (numSamplesLeft, SUB_BLOCK_MAX_SIZE);
        auto subBlock = block.getSubBlock (offset, maxChunkSize);

        auto leftBlock = subBlock.getSingleChannelBlock (static_cast<size_t> (Channel::LEFT));
        auto rightBlock = subBlock.getSingleChannelBlock (static_cast<size_t> (Channel::RIGHT));

        updateFilters (static_cast<int> (maxChunkSize));

        leftChain.process (juce::dsp::ProcessContextReplacing<float> (leftBlock));
        rightChain.process (juce::dsp::ProcessContextReplacing<float> (rightBlock));

        offset += maxChunkSize;
    }

    if (mode == EqMode::MID_SIDE)
    {
        midSideProcessor.process (juce::dsp::ProcessContextReplacing<float> (block));
    }

    outputGain.process (juce::dsp::ProcessContextReplacing<float> (block));

#ifdef USE_TEST_OSC
    for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
    {
        buffer.clear (channel, 0, buffer.getNumSamples());
    }
#endif
}

//==============================================================================
bool EqualizerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EqualizerAudioProcessor::createEditor()
{
    /* return new juce::GenericAudioProcessorEditor (*this); */
    return new EqualizerAudioProcessorEditor (*this);
}

//==============================================================================
void EqualizerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos (destData, true);
    apvts.state.writeToStream (mos);
}

void EqualizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) sizeInBytes);

    if (tree.isValid())
    {
        apvts.replaceState (tree);
        initializeFilters();
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EqualizerAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout EqualizerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    addEqModeParameterToLayout (layout);
    addGainTrimParameterToLayout (layout, "input_gain");
    addFilterParameterToLayout<ChainPositions::LOWCUT> (layout, true);
    addFilterParameterToLayout<ChainPositions::LOWSHELF> (layout, false);
    addFilterParameterToLayout<ChainPositions::PEAK1> (layout, false);
    addFilterParameterToLayout<ChainPositions::PEAK2> (layout, false);
    addFilterParameterToLayout<ChainPositions::PEAK3> (layout, false);
    addFilterParameterToLayout<ChainPositions::PEAK4> (layout, false);
    addFilterParameterToLayout<ChainPositions::HIGHSHELF> (layout, false);
    addFilterParameterToLayout<ChainPositions::HIGHCUT> (layout, true);
    addGainTrimParameterToLayout (layout, "output_gain");

    return layout;
}

void EqualizerAudioProcessor::addEqModeParameterToLayout (juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add (std::make_unique<juce::AudioParameterChoice> ("eq_mode",
                                                              "eq_mode",
                                                              juce::StringArray { "Stereo", "Dual Mono", "Mid/Side" },
                                                              0));
}

void EqualizerAudioProcessor::addGainTrimParameterToLayout (juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                                                            const juce::String& name)
{
    layout.add (std::make_unique<juce::AudioParameterFloat> (name, name, juce::NormalisableRange<float> (-18.0f, 18.0f, 0.1f), 0.0f));
}

juce::StringArray EqualizerAudioProcessor::getSlopeNames()
{
    juce::StringArray slopeNames;
    // 8 slopes of -6db/Oct each = -48db/Oct
    for (int i = 0; i < 8; ++i)
    {
        juce::String slopeName;
        slopeName << (6 + i * 6);
        slopeName << " db/Oct";
        slopeNames.add (slopeName);
    }
    return slopeNames;
}

float EqualizerAudioProcessor::getRawParameter (const juce::String& name)
{
    return apvts.getRawParameterValue (name)->load();
}

float EqualizerAudioProcessor::getRawFilterParameter (int filterIndex, Channel audioChannel, FilterInfo::FilterParam filterParameter)
{
    auto name = FilterInfo::getParameterName (filterIndex, audioChannel, filterParameter);
    return getRawParameter (name);
}

FilterParametersBase EqualizerAudioProcessor::getBaseParameters (int filterIndex, Channel audioChannel)
{
    auto frequencyParam = getRawFilterParameter (filterIndex, audioChannel, FilterInfo::FilterParam::FREQUENCY);
    auto bypassParamRaw = getRawFilterParameter (filterIndex, audioChannel, FilterInfo::FilterParam::BYPASS);
    auto bypassParam = bypassParamRaw > 0.5f;
    auto qParam = getRawFilterParameter (filterIndex, audioChannel, FilterInfo::FilterParam::Q);

    return FilterParametersBase { frequencyParam, bypassParam, qParam, getSampleRate() };
}

FilterParameters EqualizerAudioProcessor::getParametricParameters (int filterIndex, Channel audioChannel, FilterInfo::FilterType filterType)
{
    auto baseParams = getBaseParameters (filterIndex, audioChannel);
    auto gainParam = Decibel<float> (getRawFilterParameter (filterIndex, audioChannel, FilterInfo::FilterParam::GAIN));

    return FilterParameters { baseParams, filterType, gainParam };
}

HighCutLowCutParameters EqualizerAudioProcessor::getCutParameters (int filterIndex, Channel audioChannel, FilterInfo::FilterType filterType)
{
    auto baseParams = getBaseParameters (filterIndex, audioChannel);
    auto isLowCutParam = filterType == FilterInfo::FilterType::HIGHPASS;
    auto slopeParam = static_cast<int> (getRawFilterParameter (filterIndex, audioChannel, FilterInfo::FilterParam::SLOPE));

    return HighCutLowCutParameters { baseParams, slopeParam, isLowCutParam };
}

EqMode EqualizerAudioProcessor::getEqMode()
{
    return static_cast<EqMode> (getRawParameter ("eq_mode"));
}

void EqualizerAudioProcessor::initializeFilters()
{
    bool onRealTimeThread = ! juce::MessageManager::getInstanceWithoutCreating()->isThisTheMessageThread();

    auto mode = getEqMode();
    initializeCutFilter<ChainPositions::LOWCUT> (FilterInfo::FilterType::HIGHPASS, mode, onRealTimeThread);
    initializeParametricFilter<ChainPositions::LOWSHELF> (FilterInfo::FilterType::LOWSHELF, mode, onRealTimeThread);
    initializeParametricFilter<ChainPositions::PEAK1> (FilterInfo::FilterType::PEAKFILTER, mode, onRealTimeThread);
    initializeParametricFilter<ChainPositions::PEAK2> (FilterInfo::FilterType::PEAKFILTER, mode, onRealTimeThread);
    initializeParametricFilter<ChainPositions::PEAK3> (FilterInfo::FilterType::PEAKFILTER, mode, onRealTimeThread);
    initializeParametricFilter<ChainPositions::PEAK4> (FilterInfo::FilterType::PEAKFILTER, mode, onRealTimeThread);
    initializeParametricFilter<ChainPositions::HIGHSHELF> (FilterInfo::FilterType::HIGHSHELF, mode, onRealTimeThread);
    initializeCutFilter<ChainPositions::HIGHCUT> (FilterInfo::FilterType::LOWPASS, mode, onRealTimeThread);

    leftChain.reset();
    rightChain.reset();
}

void EqualizerAudioProcessor::updateParameters (EqMode mode)
{
    updateCutParameters<ChainPositions::LOWCUT> (FilterInfo::FilterType::HIGHPASS, mode);
    updateParametricParameters<ChainPositions::LOWSHELF> (FilterInfo::FilterType::LOWSHELF, mode);
    updateParametricParameters<ChainPositions::PEAK1> (FilterInfo::FilterType::PEAKFILTER, mode);
    updateParametricParameters<ChainPositions::PEAK2> (FilterInfo::FilterType::PEAKFILTER, mode);
    updateParametricParameters<ChainPositions::PEAK3> (FilterInfo::FilterType::PEAKFILTER, mode);
    updateParametricParameters<ChainPositions::PEAK4> (FilterInfo::FilterType::PEAKFILTER, mode);
    updateParametricParameters<ChainPositions::HIGHSHELF> (FilterInfo::FilterType::HIGHSHELF, mode);
    updateCutParameters<ChainPositions::HIGHCUT> (FilterInfo::FilterType::LOWPASS, mode);
}

void EqualizerAudioProcessor::updateFilters (int chunkSize)
{
    bool onRealTimeThread = ! juce::MessageManager::getInstanceWithoutCreating()->isThisTheMessageThread();

    updateFilter<ChainPositions::LOWCUT> (onRealTimeThread, chunkSize);
    updateFilter<ChainPositions::LOWSHELF> (onRealTimeThread, chunkSize);
    updateFilter<ChainPositions::PEAK1> (onRealTimeThread, chunkSize);
    updateFilter<ChainPositions::PEAK2> (onRealTimeThread, chunkSize);
    updateFilter<ChainPositions::PEAK3> (onRealTimeThread, chunkSize);
    updateFilter<ChainPositions::PEAK4> (onRealTimeThread, chunkSize);
    updateFilter<ChainPositions::HIGHSHELF> (onRealTimeThread, chunkSize);
    updateFilter<ChainPositions::HIGHCUT> (onRealTimeThread, chunkSize);
}

void EqualizerAudioProcessor::updateTrimGains()
{
    auto inputGainRaw = getRawParameter ("input_gain");
    auto outputGainRaw = getRawParameter ("output_gain");

    inputGain.setGainDecibels (inputGainRaw);
    outputGain.setGainDecibels (outputGainRaw);
}
