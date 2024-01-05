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
    {
        auto binFreq = binIndex * binWidth;
        int binX = static_cast<int> (std::floor (juce::mapFromLog10 (binFreq, 20.f, 20000.f) * fftBounds.getWidth()));
        //avoid to draw out of bounds; for binX < 0, it will be drawn at the left border of the fftBounds
        return juce::jmax (binX, 0) + fftBounds.getX();
    };

    auto toYCoordinate = [&] (float data)
    {
        return juce::jmap (data, //
                           negativeInfinity,
                           maxDb,
                           fftBounds.getBottom(),
                           fftBounds.getY());
    };

    juce::Path p;
    auto x = toXCoordinate (1);
    auto y = toYCoordinate (renderData[1]);
    p.startNewSubPath (x, y);
    auto prevX = x;

    for (size_t i = 2; i <= numBins + 1; ++i)
    {
        x = toXCoordinate (i);
        y = toYCoordinate (renderData[i]);

        if (x > prevX)
        {
            p.lineTo (x, y);
            prevX = x;
        }

        if (x > fftBounds.getRight())
        {
            break;
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
    return pathFifo.pull (path);
}
