#pragma once

#include "data/ParamListener.h"
#include "ui/AnalyzerBase.h"
#include "ui/DbScaleComponent.h"
#include "utils/MeterConstants.h"
#include "utils/PathProducer.h"
#include "utils/SingleChannelSampleFifo.h"
#include <JuceHeader.h>

#define RESPONSE_CURVE_MIN_DB -60.f
#define RESPONSE_CURVE_MAX_DB 0.f

template <typename BlockType>
struct SpectrumAnalyzer : AnalyzerBase, juce::Timer
{
    SpectrumAnalyzer (double sr,
                      SingleChannelSampleFifo<BlockType>& leftScsf,
                      SingleChannelSampleFifo<BlockType>& rightScsf,
                      juce::AudioProcessorValueTreeState& apv)
        : sampleRate { sr }, leftPathProducer { sampleRate, leftScsf }, rightPathProducer { sampleRate, rightScsf }
    {
        analyzerEnabledParamListener = std::make_unique<ParamListener> (apv,
                                                                        "Enable Analyzer",
                                                                        [this] (float value) { setActive (value > 0.5f); });

        analyzerDecayRateParamListener = std::make_unique<ParamListener> (apv,
                                                                          "Analyzer Decay Rate",
                                                                          [this] (float value) { updateDecayRate (value); });

        analyzerOrderParamListener = std::make_unique<ParamListener> (apv,
                                                                      "Analyzer Points",
                                                                      [this] (float value) { updateOrder (value); });

        updateDecayRate (apv.getRawParameterValue ("Analyzer Decay Rate")->load());
        updateOrder (apv.getRawParameterValue ("Analyzer Points")->load());
        setActive (apv.getRawParameterValue ("Enable Analyzer")->load() > 0.5f);

        addAndMakeVisible (analyzerScale);
        addAndMakeVisible (eqScale);
        animate();
    }
    void timerCallback() override
    {
        if (! active)
        {
            leftAnalyzerPath.clear();
            rightAnalyzerPath.clear();
            stopTimer();
        }
        else
        {
            while (leftPathProducer.getNumPathsAvailable() > 0)
            {
                leftPathProducer.pull (leftAnalyzerPath);
            }

            while (rightPathProducer.getNumPathsAvailable() > 0)
            {
                rightPathProducer.pull (rightAnalyzerPath);
            }

            repaint();
        }
    }

    void resized() override
    {
        AnalyzerBase::resized();
        leftPathProducer.setFFTBounds (getBoundsForFFT());
        rightPathProducer.setFFTBounds (getBoundsForFFT());

        auto bounds = getLocalBounds();
        analyzerScale.setBounds (bounds.removeFromLeft (getTextWidth()));
        eqScale.setBounds (bounds.removeFromRight (getTextWidth()));

        customizeScales (leftScaleMin, leftScaleMax, rightScaleMin, rightScaleMax, scaleDivision);
    }

    void paint (juce::Graphics& g) override
    {
        paintBackground (g);
        g.reduceClipRegion (fftBoundingBox);
        g.setColour (juce::Colours::cyan);
        g.strokePath (leftAnalyzerPath, juce::PathStrokeType (1.0f));
        g.setColour (juce::Colours::purple);
        g.strokePath (rightAnalyzerPath, juce::PathStrokeType (1.0f));
    }

    void customizeScales (int leftScaleMin, int leftScaleMax, int rightScaleMin, int rightScaleMax, int division)
    {
        leftScaleMin = leftScaleMin;
        leftScaleMax = leftScaleMax;
        rightScaleMin = rightScaleMin;
        rightScaleMax = rightScaleMax;
        scaleDivision = division;

        leftPathProducer.setDbRange (leftScaleMin, leftScaleMax);
        rightPathProducer.setDbRange (rightScaleMin, rightScaleMax);

        analyzerScale.buildBackgroundImage (scaleDivision, fftBoundingBox, leftScaleMin, leftScaleMax);
        eqScale.buildBackgroundImage (scaleDivision, fftBoundingBox, rightScaleMin, rightScaleMax);

        if (analyzerScale.getBounds().isEmpty() || eqScale.getBounds().isEmpty())
        {
            return;
        }
        repaint();
    }
    void changeSampleRate (double sr)
    {
        sampleRate = sr;
        leftPathProducer.setSampleRate (sampleRate);
        rightPathProducer.setSampleRate (sampleRate);
    }

private:
    double sampleRate;
    juce::Path leftAnalyzerPath, rightAnalyzerPath;

    PathProducer<BlockType> leftPathProducer, rightPathProducer;

    bool active { false };

    void paintBackground (juce::Graphics& g)
    {
        auto bounds = getLocalBounds();
        g.setColour (juce::Colours::aquamarine);
        g.drawRect (bounds);

        for (auto i = leftScaleMin; i <= leftScaleMax; i += scaleDivision)
        {
            g.drawHorizontalLine (juce::jmap (i, leftScaleMin, leftScaleMax, bounds.getY(), bounds.getBottom()),
                                  bounds.getX(),
                                  bounds.getRight());
        }

        std::vector<float> freqs { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };

        g.setFont (getTextHeight());
        for (auto freq : freqs)
        {
            auto x = juce::mapFromLog10 (freq, 20.f, 20000.f) * bounds.getWidth() + bounds.getX();
            g.setColour (juce::Colours::darkgrey);
            g.drawVerticalLine (x, bounds.getY(), bounds.getBottom());
            auto freqStr = freq >= 1000.0f ? juce::String (freq / 1000.f, 2) + "k" : juce::String (freq);
            //TODO: bounds?
            g.drawFittedText (freqStr, bounds, juce::Justification::topLeft, 1);
        }
    }

    void setActive (bool a)
    {
        active = a;
        if (active && ! isTimerRunning())
        {
            animate();
        }
    }

    void updateDecayRate (float dr)
    {
        leftPathProducer.setDecayRate (dr);
        rightPathProducer.setDecayRate (dr);
    }

    void updateOrder (float value)
    {
        leftPathProducer.setOrder (static_cast<FFTOrder> (static_cast<int> (value)));
        rightPathProducer.setOrder (static_cast<FFTOrder> (static_cast<int> (value)));
    }

    void animate()
    {
        startTimerHz (FRAMES_PER_SECOND);
    }

    DbScaleComponent analyzerScale, eqScale;

    std::unique_ptr<ParamListener<float>> analyzerEnabledParamListener, analyzerDecayRateParamListener, analyzerOrderParamListener;

    float leftScaleMin { RESPONSE_CURVE_MIN_DB - 30.f }, leftScaleMax { RESPONSE_CURVE_MAX_DB - 30.f },
        rightScaleMin { RESPONSE_CURVE_MIN_DB }, rightScaleMax { RESPONSE_CURVE_MAX_DB };
    int scaleDivision { 6 };
};
