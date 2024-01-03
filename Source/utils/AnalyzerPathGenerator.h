#pragma once

#include "utils/Fifo.h"
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
                       float negativeInfinity = -60.f,
                       float maxDb = 12.f);

    int getNumPathsAvailable() const;
    bool getPath (juce::Path&& path);

private:
    Fifo<juce::Path, 50> pathFifo;
};
