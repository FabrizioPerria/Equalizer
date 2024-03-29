#include "utils/AnalyzerPathGenerator.h"

void AnalyzerPathGenerator::generatePath (const std::vector<float>& renderData,
                                          juce::Rectangle<float> fftBounds,
                                          int fftSize,
                                          float binWidth,
                                          float negativeInfinity,
                                          float maxDb)
{
    size_t numBins = static_cast<size_t> (fftSize / 2);

    auto startX = fftBounds.toNearestInt().getX();
    auto toXCoordinate = [&] (size_t binIndex)
    {
        auto binFreq = binIndex * binWidth;
        int binX = static_cast<int> (std::floor (juce::mapFromLog10 (binFreq, 20.f, 20000.f) * fftBounds.getWidth()));
        //avoid to draw out of bounds; for binX < 0, it will be drawn at the left border of the fftBounds
        return juce::jmax (binX, 0) + startX;
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

    if (x > startX)
    {
        auto index = 20.f / binWidth;
        x = startX;
        auto gain = index * renderData[1] + (1 - index) * renderData[0];
        y = toYCoordinate (gain);
    }

    p.startNewSubPath (x, y);
    auto prevX = x;
    auto minY = y;
    auto findMin = false;

    for (size_t i = 2; i <= numBins; ++i)
    {
        x = toXCoordinate (i);
        y = toYCoordinate (renderData[i]);

        if (findMin && y < minY)
        {
            minY = y;
        }

        if (x > prevX)
        {
            p.lineTo (x, findMin ? minY : y);
            prevX = x;
            findMin = false;
        }
        else if (! findMin)
        {
            findMin = true;
            minY = y;
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

bool AnalyzerPathGenerator::getPath (juce::Path& path)
{
    return pathFifo.pull (path);
}
