#pragma once

#include "utils/Fifo.h"
#include "utils/MeterConstants.h"
#include <JuceHeader.h>

struct AnalyzerPathGenerator
{
    /*
     converts 'renderData[]' into a juce::Path
     */
    void generatePath (const std::vector<float>& renderData,
                       juce::Rectangle<float> fftBounds,
                       int fftSize,
                       float binWidth,
                       float negativeInfinity = NEGATIVE_INFINITY,
                       float maxDb = MAX_DECIBELS);

    int getNumPathsAvailable() const;
    bool getPath (juce::Path&& path);

private:
    Fifo<juce::Path, 50> pathFifo;
};
