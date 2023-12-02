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

    updateFilters (apvts, leftChain, rightChain, sampleRate);
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
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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
    auto leftBlock = block.getSingleChannelBlock (0);
    auto rightBlock = block.getSingleChannelBlock (1);

    updateFilters (apvts, leftChain, rightChain, getSampleRate());

    leftChain.process (juce::dsp::ProcessContextReplacing<float> (leftBlock));
    rightChain.process (juce::dsp::ProcessContextReplacing<float> (rightBlock));
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
	juce::MemoryOutputStream mos(destData, true);
	apvts.state.writeToStream(mos);
}

void EqualizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	auto tree = juce::ValueTree::readFromData(data, (size_t)sizeInBytes);

	if (tree.isValid())
	{
		apvts.replaceState(tree);
        updateFilters (apvts, leftChain, rightChain, getSampleRate());
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

    for (int i = 0; i < NUM_FILTERS; ++i)
    {
        auto name = FilterInfo::getParameterName (i, FilterInfo::FilterParam::BYPASS);
        layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { name, 1 }, name, false));

        name = FilterInfo::getParameterName (i, FilterInfo::FilterParam::FREQUENCY);
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { name, 1 }, name, juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.25f), 20.0f));

        name = FilterInfo::getParameterName (i, FilterInfo::FilterParam::Q);
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { name, 1 }, name, juce::NormalisableRange<float> (0.1f, 10.0f, 0.1f), 1.0f));

        name = FilterInfo::getParameterName (i, FilterInfo::FilterParam::GAIN);
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { name, 1 }, name, juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f));

        name = FilterInfo::getParameterName (i, FilterInfo::FilterParam::FILTER_TYPE);
        layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { name, 1 },
                                                                  name,
                                                                  FilterInfo::getFilterTypeNames(),
                                                                  static_cast<int> (FilterInfo::FilterType::ALLPASS)));
    }

    return layout;
}