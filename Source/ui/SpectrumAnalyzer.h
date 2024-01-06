#pragma once

#include "data/ParamListener.h"
#include "ui/AnalyzerBase.h"
#include "ui/DbScaleComponent.h"
#include "utils/MeterConstants.h"
#include "utils/PathProducer.h"
#include "utils/SingleChannelSampleFifo.h"
#include <JuceHeader.h>

#define RESPONSE_CURVE_MIN_DB -30.f
#define RESPONSE_CURVE_MAX_DB 30.f

template <typename BlockType>
struct SpectrumAnalyzer : AnalyzerBase, juce::Timer
{
    SpectrumAnalyzer (double sr,
                      SingleChannelSampleFifo<BlockType>& leftScsf,
                      SingleChannelSampleFifo<BlockType>& rightScsf,
                      juce::AudioProcessorValueTreeState& apv)
        : sampleRate { sr }, leftPathProducer { sampleRate, leftScsf }, rightPathProducer { sampleRate, rightScsf }
    {
        auto safePtr = juce::Component::SafePointer<SpectrumAnalyzer<BlockType>> (this);

        analyzerEnabledParamListener = std::make_unique<ParamListener<float>> (apv.getParameter ("Enable Analyzer"),
                                                                               [safePtr] (float value)
                                                                               {
                                                                                   if (auto* comp = safePtr.getComponent())
                                                                                       comp->setActive (value > 0.5f);
                                                                               });

        analyzerDecayRateParamListener = std::make_unique<ParamListener<float>> (apv.getParameter ("Analyzer Decay Rate"),
                                                                                 [safePtr] (float value)
                                                                                 {
                                                                                     if (auto* comp = safePtr.getComponent())
                                                                                         comp->updateDecayRate (value);
                                                                                 });

        analyzerOrderParamListener = std::make_unique<ParamListener<float>> (apv.getParameter ("Analyzer Points"),
                                                                             [safePtr] (float value)
                                                                             {
                                                                                 if (auto* comp = safePtr.getComponent())
                                                                                     comp->updateOrder (value);
                                                                             });

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
            while (leftPathProducer.getNumAvailableForReading() > 0)
            {
                leftPathProducer.pull (leftAnalyzerPath);
            }

            while (rightPathProducer.getNumAvailableForReading() > 0)
            {
                rightPathProducer.pull (rightAnalyzerPath);
            }
        }
        repaint();
    }

    void resized() override
    {
        AnalyzerBase::resized();
        leftPathProducer.setFFTRectBounds (fftBoundingBox.toFloat());
        rightPathProducer.setFFTRectBounds (fftBoundingBox.toFloat());

        auto bounds = getLocalBounds();
        analyzerScale.setBounds (bounds.removeFromLeft (getTextWidth() * 1.5f));
        eqScale.setBounds (bounds.removeFromRight (getTextWidth() * 1.5f));

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

        leftPathProducer.changePathRange (leftScaleMin, leftScaleMax);
        rightPathProducer.changePathRange (leftScaleMin, leftScaleMax);

        analyzerScale.buildBackgroundImage (scaleDivision, fftBoundingBox, leftScaleMin, leftScaleMax);
        eqScale.buildBackgroundImage (scaleDivision, fftBoundingBox, rightScaleMin, rightScaleMax);

        if (! getLocalBounds().isEmpty())
        {
            repaint();
        }
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
        g.setColour (juce::Colours::aquamarine);
        g.drawRect (getLocalBounds());

        auto scale = leftScaleMax;
        while (scale >= leftScaleMin)
        {
            auto y = juce::jmap (scale, leftScaleMin, leftScaleMax, fftBoundingBox.toFloat().getBottom(), fftBoundingBox.toFloat().getY());

            g.drawHorizontalLine (y, fftBoundingBox.getX(), fftBoundingBox.getRight());

            scale -= scaleDivision;
            //we change the color after printing the first line to keep it highlighted
            g.setColour (juce::Colours::darkgrey);
        }

        std::vector<float> freqs { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };

        g.setFont (10);
        auto textBound = fftBoundingBox.withWidth (2 * getTextWidth()).withHeight (getTextHeight()).translated (0, 2);
        for (auto freq : freqs)
        {
            auto x = juce::mapFromLog10 (freq, 20.f, 20000.f) * fftBoundingBox.getWidth() + fftBoundingBox.getX();
            auto freqStr = freq >= 1000.0f ? juce::String (freq / 1000.f, 0) + "k" : juce::String (freq);
            if (freq == 20000.0f)
            {
                // don't draw 20k on top of the eqScale!
                x -= getTextWidth();
            }
            else if (freq == 20.0f)
            {
                // give the first tick the unit
                freqStr = freqStr + "Hz";
            }
            else
            {
                // don't draw first and last line
                g.drawVerticalLine (x, fftBoundingBox.getY() + 1, fftBoundingBox.getBottom());
            }
            g.drawFittedText (freqStr, textBound.withX (x + 2), juce::Justification::topLeft, 1);
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
        leftPathProducer.changeOrder (static_cast<FFTOrder> (static_cast<int> (value) + 11));
        rightPathProducer.changeOrder (static_cast<FFTOrder> (static_cast<int> (value) + 11));
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
