#include "ui/ResponseCurveComponent.h"
#include "PluginProcessor.h"
#include "utils/ChainHelpers.h"
#include "utils/EqParam.h"
#include "utils/MeterConstants.h"

ResponseCurveComponent::ResponseCurveComponent (double sr, juce::AudioProcessorValueTreeState& apvts) : apvts (&apvts), sampleRate (sr)
{
    auto bindFunc = [this] { refreshParams(); };
    allParamsListener = std::make_unique<AllParamsListener> (&apvts, bindFunc);
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 1024;
    spec.numChannels = 1;

    leftChain.prepare (spec);
    rightChain.prepare (spec);
}

void ResponseCurveComponent::paint (juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState saveState (g);
    g.reduceClipRegion (fftBoundingBox);

    g.setColour (juce::Colours::yellow);

    juce::PathStrokeType strokeType (1.f);
    g.strokePath (leftResponseCurve, strokeType);
    auto getProcessingMode = static_cast<EqMode> (apvts->getRawParameterValue ("eq_mode")->load());
    if (getProcessingMode != EqMode::STEREO)
    {
        g.setColour (juce::Colours::hotpink);
        g.strokePath (rightResponseCurve, strokeType);
    }
    // when saveState goes out of scope, the clip region is restored
}

void ResponseCurveComponent::resized()
{
    AnalyzerBase::resized();
    buildNewResponseCurves();
}

void ResponseCurveComponent::refreshParams()
{
    buildNewResponseCurves();
    repaint();
}

void ResponseCurveComponent::buildNewResponseCurves()
{
    updateChainParameters();
    auto w = static_cast<size_t> (fftBoundingBox.getWidth());

    std::vector<float> pathData;
    pathData.resize (w, NEGATIVE_INFINITY);

    buildNewResponseCurve (pathData, leftChain);
    createResponseCurve (leftResponseCurve, pathData);
    auto getProcessingMode = static_cast<EqMode> (apvts->getRawParameterValue ("eq_mode")->load());
    if (getProcessingMode != EqMode::STEREO)
    {
        buildNewResponseCurve (pathData, rightChain);
        createResponseCurve (rightResponseCurve, pathData);
    }
}
void ResponseCurveComponent::updateChainParameters()
{
    ChainHelpers::initializeChains (leftChain, rightChain, sampleRate, *apvts);
}

void ResponseCurveComponent::buildNewResponseCurve (std::vector<float>& path, EqualizerAudioProcessor::MonoChain& chain)
{
    path.assign (path.size(), NEGATIVE_INFINITY);

    for (auto x = 0; x < fftBoundingBox.getWidth(); ++x)
    {
        double mag = 1;
        auto freq = juce::mapToLog10 (static_cast<double> (x) / fftBoundingBox.getWidth(), 20.0, 20000.0);

        mag *= chain.template get<0>().isBypassed() ? 1.0 : chain.template get<0>().getCutFilterMagnitudeForFrequency (freq);
        mag *= chain.template get<1>().isBypassed() ? 1.0 : chain.template get<1>().getParametricFilterMagnitudeForFrequency (freq);
        mag *= chain.template get<2>().isBypassed() ? 1.0 : chain.template get<2>().getParametricFilterMagnitudeForFrequency (freq);
        mag *= chain.template get<3>().isBypassed() ? 1.0 : chain.template get<3>().getParametricFilterMagnitudeForFrequency (freq);
        mag *= chain.template get<4>().isBypassed() ? 1.0 : chain.template get<4>().getParametricFilterMagnitudeForFrequency (freq);
        mag *= chain.template get<5>().isBypassed() ? 1.0 : chain.template get<5>().getParametricFilterMagnitudeForFrequency (freq);
        mag *= chain.template get<6>().isBypassed() ? 1.0 : chain.template get<6>().getParametricFilterMagnitudeForFrequency (freq);
        mag *= chain.template get<7>().isBypassed() ? 1.0 : chain.template get<7>().getCutFilterMagnitudeForFrequency (freq);

        path[x] = juce::Decibels::gainToDecibels (static_cast<float> (mag));
    }
}

void ResponseCurveComponent::createResponseCurve (juce::Path& path, const std::vector<float>& data)
{
    path.clear();
    if (data.empty())
    {
        return;
    }

    auto startX = fftBoundingBox.getX();

    auto toYCoordinate = [this] (float gain)
    {
        auto bBox = fftBoundingBox.toFloat();
        return juce::jmap (gain,
                           static_cast<float> (RESPONSE_CURVE_MIN_DB),
                           static_cast<float> (RESPONSE_CURVE_MAX_DB),
                           bBox.getBottom(),
                           bBox.getY());
    };

    path.startNewSubPath (startX, toYCoordinate (data[0]));

    for (size_t i = 1; i < data.size(); ++i)
    {
        path.lineTo (i + startX, toYCoordinate (data[i]));
    }
}
