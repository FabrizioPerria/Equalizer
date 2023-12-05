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

    updateFilters();
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

    updateFilters();

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

    for (int i = 0; i < NUM_FILTERS; ++i)
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

        name = FilterInfo::getParameterName (i, FilterInfo::FilterParam::GAIN);
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { name, 1 },
                                                                 name,
                                                                 juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f),
                                                                 0.0f));

        name = FilterInfo::getParameterName (i, FilterInfo::FilterParam::FILTER_TYPE);
        layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { name, 1 },
                                                                  name,
                                                                  getFilterTypeNames(),
                                                                  static_cast<int> (FilterInfo::FilterType::ALLPASS)));
    }

    return layout;
}

const std::map<FilterInfo::FilterType, juce::String> EqualizerAudioProcessor::filterTypeMap = {
    { FilterInfo::FilterType::FIRST_ORDER_LOWPASS, "First Order Lowpass" },
    { FilterInfo::FilterType::FIRST_ORDER_HIGHPASS, "First Order Highpass" },
    { FilterInfo::FilterType::FIRST_ORDER_ALLPASS, "First Order Allpass" },
    { FilterInfo::FilterType::LOWPASS, "Lowpass" },
    { FilterInfo::FilterType::HIGHPASS, "Highpass" },
    { FilterInfo::FilterType::BANDPASS, "Bandpass" },
    { FilterInfo::FilterType::NOTCH, "Notch" },
    { FilterInfo::FilterType::ALLPASS, "Allpass" },
    { FilterInfo::FilterType::LOWSHELF, "Lowshelf" },
    { FilterInfo::FilterType::HIGHSHELF, "Highshelf" },
    { FilterInfo::FilterType::PEAKFILTER, "Peakfilter" },
};

FilterInfo::FilterType EqualizerAudioProcessor::getFilterType (int filterIndex)
{
    auto name = FilterInfo::getParameterName (filterIndex, FilterInfo::FilterParam::FILTER_TYPE);
    auto filterTypeParam = apvts.getRawParameterValue (name);
    return static_cast<FilterInfo::FilterType> (filterTypeParam->load());
}

juce::String EqualizerAudioProcessor::getFilterTypeName (FilterInfo::FilterType filterType)
{
    auto it = filterTypeMap.find (filterType);
    if (it != filterTypeMap.end())
    {
        return it->second;
    }
    return "Unknown";
}

juce::StringArray EqualizerAudioProcessor::getFilterTypeNames()
{
    juce::StringArray names;
    for (auto& it : filterTypeMap)
    {
        names.add (it.second);
    }
    return names;
}

float EqualizerAudioProcessor::getRawParameter (int filterIndex, FilterInfo::FilterParam filterParameter)
{
    auto name = FilterInfo::getParameterName (filterIndex, filterParameter);
    return apvts.getRawParameterValue (name)->load();
}

FilterParametersBase EqualizerAudioProcessor::getBaseParameters (int filterIndex)
{
    auto bypassParamRaw = getRawParameter (filterIndex, FilterInfo::FilterParam::BYPASS);
    auto bypassParam = bypassParamRaw > 0.5f;

    auto frequencyParam = getRawParameter (filterIndex, FilterInfo::FilterParam::FREQUENCY);

    auto qParam = getRawParameter (filterIndex, FilterInfo::FilterParam::Q);

    return FilterParametersBase { frequencyParam, bypassParam, qParam, getSampleRate() };
}

FilterParameters EqualizerAudioProcessor::getParametricParameters (int filterIndex, FilterInfo::FilterType filterType)
{
    auto baseParams = getBaseParameters (filterIndex);

    auto gainParam = getRawParameter (filterIndex, FilterInfo::FilterParam::GAIN);

    return FilterParameters { baseParams, filterType, gainParam };
}

HighCutLowCutParameters EqualizerAudioProcessor::getCutParameters (int filterIndex, FilterInfo::FilterType filterType)
{
    auto baseParams = getBaseParameters (filterIndex);
    auto isLowCutParam = filterType == FilterInfo::FilterType::HIGHPASS;
    return HighCutLowCutParameters { baseParams, 1, isLowCutParam };
}

bool EqualizerAudioProcessor::needsParametricParams (FilterInfo::FilterType type)
{
    switch (type)
    {
        case FilterInfo::FilterType::FIRST_ORDER_LOWPASS:
        case FilterInfo::FilterType::FIRST_ORDER_HIGHPASS:
        case FilterInfo::FilterType::FIRST_ORDER_ALLPASS:
        case FilterInfo::FilterType::BANDPASS:
        case FilterInfo::FilterType::NOTCH:
        case FilterInfo::FilterType::LOWSHELF:
        case FilterInfo::FilterType::HIGHSHELF:
        case FilterInfo::FilterType::ALLPASS:
        case FilterInfo::FilterType::PEAKFILTER:
            return true;
        case FilterInfo::FilterType::LOWPASS:
        case FilterInfo::FilterType::HIGHPASS:
            return false;
        default:
            jassertfalse;
            return false;
    }
}

void EqualizerAudioProcessor::setBypassed (MonoFilter& filter, int filterIndex, bool bypassed)
{
    jassert (filterIndex >= 0 && filterIndex < NUM_FILTERS);
    switch (filterIndex)
    {
        case 0:
            filter.setBypassed<0> (bypassed);
            break;

        default:
            jassertfalse;
            break;
    }
}

void EqualizerAudioProcessor::updateCoefficients (MonoFilter& filter, int filterIndex, Coefficients coefficients)
{
    jassert (filterIndex >= 0 && filterIndex < NUM_FILTERS);
    switch (filterIndex)
    {
        case 0:
            *filter.get<0>().coefficients = *coefficients;
            break;

        default:
            jassertfalse;
            break;
    }
}

void EqualizerAudioProcessor::updateFilters()
{
    for (int i = 0; i < NUM_FILTERS; ++i)
    {
        auto filterType = getFilterType (i);

        if (needsParametricParams (filterType))
        {
            updateFilter (i, oldFilterParams[i], getParametricParameters (i, filterType));
        }
        else
        {
            updateFilter (i, oldHighCutLowCutParams[i], getCutParameters (i, filterType));
        }
    }
}
