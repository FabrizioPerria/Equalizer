#include "utils/AnalyzerPathGenerator.h"

void AnalyzerPathGenerator::generatePath (const std::vector<float>& renderData,
                                          juce::Rectangle<float> fftBounds,
                                          int fftSize,
                                          float binWidth,
                                          float negativeInfinity,
                                          float maxDb)
{
    auto numBins = fftSize / 2;

    auto toXCoordinate = [&] (int binIndex)
    { return juce::jmap (binIndex * binWidth, 20.f, 20000.f, fftBounds.getX(), fftBounds.getRight()); };
    auto toYCoordinate = [&] (float data) { return juce::jmap (data, negativeInfinity, maxDb, fftBounds.getBottom(), fftBounds.getY()); };

    juce::Path p;
    auto x = toXCoordinate (1);
    auto y = toYCoordinate (renderData[1]);
    p.startNewSubPath (x, y);
    auto prevX = x;

    for (auto i = 2; i < numBins + 1; ++i)
    {
        auto x = toXCoordinate (i);
        auto y = toYCoordinate (renderData[i]);

        if (x - prevX > 1)
        {
            p.lineTo (x, y);
        }
    }
    pathFifo.push (p);
}

int AnalyzerPathGenerator::getNumPathsAvailable() const
{
    return pathFifo.getNumAvailableForReading();
}

bool AnalyzerPathGenerator::getPath (juce::Path&& path)
{
    return pathFifo.exchange (std::move (path));
}
