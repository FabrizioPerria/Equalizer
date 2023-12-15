/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
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

    initializeFilters (sampleRate);
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

    performPreLoopUpdate (getSampleRate());

    auto block = juce::dsp::AudioBlock<float> (buffer);

    const size_t SUB_BLOCK_MAX_SIZE = 32;
    for (size_t offset = 0; offset < block.getNumSamples();)
    {
        auto numSamplesLeft = block.getNumSamples() - offset;
        auto maxChunkSize = juce::jmin (numSamplesLeft, SUB_BLOCK_MAX_SIZE);
        auto subBlock = block.getSubBlock (offset, maxChunkSize);

        auto leftBlock = subBlock.getSingleChannelBlock (static_cast<size_t> (Channel::LEFT));
        auto rightBlock = subBlock.getSingleChannelBlock (static_cast<size_t> (Channel::RIGHT));

        performInnerLoopUpdate (getSampleRate(), maxChunkSize);

        leftChain.process (juce::dsp::ProcessContextReplacing<float> (leftBlock));
        rightChain.process (juce::dsp::ProcessContextReplacing<float> (rightBlock));

        offset += maxChunkSize;
    }
}

//==============================================================================
bool EqualizerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EqualizerAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
    /* return new EqualizerAudioProcessorEditor (*this); */
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
        updateFilters();
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

    for (int i = 0; i < static_cast<int> (ChainPositions::NUM_FILTERS); ++i)
    {
        auto name = FilterInfo::getParameterName (i, FilterInfo::FilterParam::BYPASS);
        layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { name, 1 }, //
                                                                name,
                                                                false));

        name = FilterInfo::getParameterName (i, FilterInfo::FilterParam::FREQUENCY);
        auto range = juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.25f);
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { name, 1 }, //
                                                                 name,
                                                                 range,
                                                                 20.0f));

        name = FilterInfo::getParameterName (i, FilterInfo::FilterParam::Q);
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { name, 1 },
                                                                 name,
                                                                 juce::NormalisableRange<float> (0.1f, 10.0f, 0.1f),
                                                                 1.0f));

        auto currentPosition = static_cast<ChainPositions> (i);
        if (currentPosition == ChainPositions::HIGHCUT || currentPosition == ChainPositions::LOWCUT)
        {
            name = FilterInfo::getParameterName (i, FilterInfo::FilterParam::SLOPE);
            layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { name, 1 }, //
                                                                      name,
                                                                      getSlopeNames(),
                                                                      0));
        }
        else
        {
            name = FilterInfo::getParameterName (i, FilterInfo::FilterParam::GAIN);
            layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { name, 1 },
                                                                     name,
                                                                     juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f),
                                                                     0.0f));
        }
    }

    return layout;
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

float EqualizerAudioProcessor::getRawParameter (int filterIndex, FilterInfo::FilterParam filterParameter)
{
    auto name = FilterInfo::getParameterName (filterIndex, filterParameter);
    return apvts.getRawParameterValue (name)->load();
}

FilterParametersBase EqualizerAudioProcessor::getBaseParameters (int filterIndex)
{
    auto frequencyParam = getRawParameter (filterIndex, FilterInfo::FilterParam::FREQUENCY);
    auto bypassParamRaw = getRawParameter (filterIndex, FilterInfo::FilterParam::BYPASS);
    auto bypassParam = bypassParamRaw > 0.5f;
    auto qParam = getRawParameter (filterIndex, FilterInfo::FilterParam::Q);

    return FilterParametersBase { frequencyParam, bypassParam, qParam, getSampleRate() };
}

FilterParameters EqualizerAudioProcessor::getParametricParameters (int filterIndex, FilterInfo::FilterType filterType)
{
    auto baseParams = getBaseParameters (filterIndex);
    auto gainParam = Decibel<float> (getRawParameter (filterIndex, FilterInfo::FilterParam::GAIN));

    return FilterParameters { baseParams, filterType, gainParam };
}

HighCutLowCutParameters EqualizerAudioProcessor::getCutParameters (int filterIndex, FilterInfo::FilterType filterType)
{
    auto baseParams = getBaseParameters (filterIndex);
    auto isLowCutParam = filterType == FilterInfo::FilterType::HIGHPASS;
    auto slopeParam = static_cast<int> (getRawParameter (filterIndex, FilterInfo::FilterParam::SLOPE));

    return HighCutLowCutParameters { baseParams, slopeParam, isLowCutParam };
}

void EqualizerAudioProcessor::initializeFilters (double sampleRate)
{
    // am i on the audio thread here?
    bool onRealTimeThread = ! juce::MessageManager::getInstanceWithoutCreating()->isThisTheMessageThread();
    auto rampTime = 0.05f;
    //
    // get filter params from apvts and initialize filters
    auto lowPassParams = getCutParameters (0, FilterInfo::FilterType::LOWPASS);
    leftChain.get<0>().initialize (lowPassParams, rampTime, onRealTimeThread, sampleRate);
    rightChain.get<0>().initialize (lowPassParams, rampTime, onRealTimeThread, sampleRate);

    auto lowShelfParams = getParametricParameters (1, FilterInfo::FilterType::LOWSHELF);
    leftChain.get<1>().initialize (lowShelfParams, rampTime, onRealTimeThread, sampleRate);
    rightChain.get<1>().initialize (lowShelfParams, rampTime, onRealTimeThread, sampleRate);

    auto peakParams0 = getParametricParameters (2, FilterInfo::FilterType::PEAKFILTER);
    leftChain.get<2>().initialize (peakParams0, rampTime, onRealTimeThread, sampleRate);
    rightChain.get<2>().initialize (peakParams0, rampTime, onRealTimeThread, sampleRate);

    auto peakParams1 = getParametricParameters (3, FilterInfo::FilterType::PEAKFILTER);
    leftChain.get<3>().initialize (peakParams1, rampTime, onRealTimeThread, sampleRate);
    rightChain.get<3>().initialize (peakParams1, rampTime, onRealTimeThread, sampleRate);

    auto peakParams2 = getParametricParameters (4, FilterInfo::FilterType::PEAKFILTER);
    leftChain.get<4>().initialize (peakParams2, rampTime, onRealTimeThread, sampleRate);
    rightChain.get<4>().initialize (peakParams2, rampTime, onRealTimeThread, sampleRate);

    auto peakParams3 = getParametricParameters (5, FilterInfo::FilterType::PEAKFILTER);
    leftChain.get<5>().initialize (peakParams3, rampTime, onRealTimeThread, sampleRate);
    rightChain.get<5>().initialize (peakParams3, rampTime, onRealTimeThread, sampleRate);

    auto highShelfParams = getParametricParameters (6, FilterInfo::FilterType::HIGHSHELF);
    leftChain.get<6>().initialize (highShelfParams, rampTime, onRealTimeThread, sampleRate);
    rightChain.get<6>().initialize (highShelfParams, rampTime, onRealTimeThread, sampleRate);

    auto highPassParams = getCutParameters (7, FilterInfo::FilterType::HIGHPASS);
    leftChain.get<7>().initialize (highPassParams, rampTime, onRealTimeThread, sampleRate);
    rightChain.get<7>().initialize (highPassParams, rampTime, onRealTimeThread, sampleRate);
}

void EqualizerAudioProcessor::performPreLoopUpdate (const double sampleRate)
{
    auto lowPassParams = getCutParameters (0, FilterInfo::FilterType::LOWPASS);
    leftChain.get<0>().performPreloopUpdate (lowPassParams);
    rightChain.get<0>().performPreloopUpdate (lowPassParams);

    auto lowShelfParams = getParametricParameters (1, FilterInfo::FilterType::LOWSHELF);
    leftChain.get<1>().performPreloopUpdate (lowShelfParams);
    rightChain.get<1>().performPreloopUpdate (lowShelfParams);

    auto peakParams0 = getParametricParameters (2, FilterInfo::FilterType::PEAKFILTER);
    leftChain.get<2>().performPreloopUpdate (peakParams0);
    rightChain.get<2>().performPreloopUpdate (peakParams0);

    auto peakParams1 = getParametricParameters (3, FilterInfo::FilterType::PEAKFILTER);
    leftChain.get<3>().performPreloopUpdate (peakParams1);
    rightChain.get<3>().performPreloopUpdate (peakParams1);

    auto peakParams2 = getParametricParameters (4, FilterInfo::FilterType::PEAKFILTER);
    leftChain.get<4>().performPreloopUpdate (peakParams2);
    rightChain.get<4>().performPreloopUpdate (peakParams2);

    auto peakParams3 = getParametricParameters (5, FilterInfo::FilterType::PEAKFILTER);
    leftChain.get<5>().performPreloopUpdate (peakParams3);
    rightChain.get<5>().performPreloopUpdate (peakParams3);

    auto highShelfParams = getParametricParameters (6, FilterInfo::FilterType::HIGHSHELF);
    leftChain.get<6>().performPreloopUpdate (highShelfParams);
    rightChain.get<6>().performPreloopUpdate (highShelfParams);

    auto highPassParams = getCutParameters (7, FilterInfo::FilterType::HIGHPASS);
    leftChain.get<7>().performPreloopUpdate (highPassParams);
    rightChain.get<7>().performPreloopUpdate (highPassParams);
}

void EqualizerAudioProcessor::performInnerLoopUpdate (const size_t chunkSize)
{
    bool onRealTimeThread = ! juce::MessageManager::getInstanceWithoutCreating()->isThisTheMessageThread();

    leftChain.get<0>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);
    rightChain.get<0>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);

    leftChain.get<1>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);
    rightChain.get<1>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);

    leftChain.get<2>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);
    rightChain.get<2>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);

    leftChain.get<3>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);
    rightChain.get<3>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);

    leftChain.get<4>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);
    rightChain.get<4>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);

    leftChain.get<5>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);
    rightChain.get<5>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);

    leftChain.get<6>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);
    rightChain.get<6>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);

    leftChain.get<7>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);
    rightChain.get<7>().performInnerLoopFilterUpdate (onRealTimeThread, chunkSize);
}
