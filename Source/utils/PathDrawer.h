#pragma once

#include <JuceHeader.h>

struct PathDrawer
{
    template <typename BoundType>
    static void drawPowerSymbol (juce::Graphics& g, const BoundType& bounds)
    {
        juce::Path powerSymbol;
        auto size = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.6f;

        auto r = bounds.withSizeKeepingCentre (size, size);
        float angInRadians = juce::degreesToRadians (30.0f);

        powerSymbol.addCentredArc (r.getCentreX(),
                                   r.getCentreY(),
                                   size / 2,
                                   size / 2,
                                   0.0f,
                                   angInRadians,
                                   juce::MathConstants<float>::twoPi - angInRadians,
                                   true);

        powerSymbol.startNewSubPath (r.getCentreX(), r.getY());
        powerSymbol.lineTo (r.getCentre());

        g.strokePath (powerSymbol, juce::PathStrokeType (1.0f));
    }

    template <typename BoundType>
    static void drawCutFilterSymbol (juce::Graphics& g,
                                     const BoundType& bounds,
                                     juce::AffineTransform& transform,
                                     bool enableLeft,
                                     bool enableRight)
    {
        auto anchorPoints = createAnchorPoints (bounds);

        juce::Path cutFilterSymbolLeft;
        setColorByEnable (g, enableLeft);
        cutFilterSymbolLeft.startNewSubPath (anchorPoints.startX, anchorPoints.bottomY);
        cutFilterSymbolLeft.lineTo (anchorPoints.leftX, anchorPoints.topY);
        cutFilterSymbolLeft.lineTo (anchorPoints.centerX, anchorPoints.topY);
        g.strokePath (cutFilterSymbolLeft, juce::PathStrokeType (1.0f), transform);

        juce::Path cutFilterSymbolRight;
        setColorByEnable (g, enableRight);
        cutFilterSymbolRight.startNewSubPath (anchorPoints.centerX, anchorPoints.topY);
        cutFilterSymbolRight.lineTo (anchorPoints.endX, anchorPoints.topY);
        g.strokePath (cutFilterSymbolRight, juce::PathStrokeType (1.0f), transform);
    }

    template <typename BoundType>
    static void drawShelfFilterSymbol (juce::Graphics& g,
                                       const BoundType bounds,
                                       juce::AffineTransform& transform,
                                       bool enableLeft,
                                       bool enableRight)
    {
        auto anchorPoints = createAnchorPoints (bounds);

        juce::Path shelfFilterSymbolLeft;
        setColorByEnable (g, enableLeft);
        shelfFilterSymbolLeft.startNewSubPath (anchorPoints.startX, anchorPoints.bottomY);
        shelfFilterSymbolLeft.lineTo (anchorPoints.leftX, anchorPoints.bottomY);
        shelfFilterSymbolLeft.lineTo (anchorPoints.centerX, anchorPoints.centerY);
        shelfFilterSymbolLeft.startNewSubPath (anchorPoints.startX, anchorPoints.topY);
        shelfFilterSymbolLeft.lineTo (anchorPoints.leftX, anchorPoints.topY);
        shelfFilterSymbolLeft.lineTo (anchorPoints.centerX, anchorPoints.centerY);
        g.strokePath (shelfFilterSymbolLeft, juce::PathStrokeType (1.0f), transform);

        juce::Path shelfFilterSymbolRight;
        setColorByEnable (g, enableRight);
        shelfFilterSymbolRight.startNewSubPath (anchorPoints.centerX, anchorPoints.centerY);
        shelfFilterSymbolRight.lineTo (anchorPoints.endX, anchorPoints.centerY);
        g.strokePath (shelfFilterSymbolRight, juce::PathStrokeType (1.0f), transform);
    }

    template <typename BoundType>
    static void drawPeakFilterSymbol (juce::Graphics& g, const BoundType bounds, bool enableLeft, bool enableRight)
    {
        auto anchorPoints = createAnchorPoints (bounds);

        juce::Path peakFilterSymbolLeft;
        setColorByEnable (g, enableLeft);
        peakFilterSymbolLeft.startNewSubPath (anchorPoints.startX, anchorPoints.centerY);
        peakFilterSymbolLeft.lineTo (anchorPoints.leftX, anchorPoints.centerY);
        peakFilterSymbolLeft.lineTo (anchorPoints.centerX, anchorPoints.topY);
        peakFilterSymbolLeft.startNewSubPath (anchorPoints.leftX, anchorPoints.centerY);
        peakFilterSymbolLeft.lineTo (anchorPoints.centerX, anchorPoints.bottomY);
        g.strokePath (peakFilterSymbolLeft, juce::PathStrokeType (1.0f));

        juce::Path peakFilterSymbolRight;
        setColorByEnable (g, enableRight);
        peakFilterSymbolRight.startNewSubPath (anchorPoints.centerX, anchorPoints.topY);
        peakFilterSymbolRight.lineTo (anchorPoints.rightX, anchorPoints.centerY);
        peakFilterSymbolRight.lineTo (anchorPoints.endX, anchorPoints.centerY);
        peakFilterSymbolRight.startNewSubPath (anchorPoints.centerX, anchorPoints.bottomY);
        peakFilterSymbolRight.lineTo (anchorPoints.rightX, anchorPoints.centerY);
        g.strokePath (peakFilterSymbolRight, juce::PathStrokeType (1.0f));
    }

private:
    struct AnchorPoints
    {
        float startX, topY;
        float leftX;
        float centerX, centerY;
        float rightX;
        float endX, bottomY;
    };

    static AnchorPoints createAnchorPoints (const juce::Rectangle<float>& bounds)
    {
        AnchorPoints anchorPoints;

        auto x = bounds.getX();
        auto y = bounds.getY();
        auto width = bounds.getWidth();
        auto height = bounds.getHeight();

        anchorPoints.startX = x + 2 * width / 8;
        anchorPoints.leftX = x + 3 * width / 8;
        anchorPoints.centerX = x + 4 * width / 8;
        anchorPoints.rightX = x + 5 * width / 8;
        anchorPoints.endX = x + 6 * width / 8;
        anchorPoints.topY = y + height / 4;
        anchorPoints.centerY = y + 2 * height / 4;
        anchorPoints.bottomY = y + 3 * height / 4;

        return anchorPoints;
    }

    static void setColorByEnable (juce::Graphics& g, bool enable)
    {
        if (enable)
        {
            g.setColour (juce::Colours::aquamarine);
        }
        else
        {
            g.setColour (juce::Colours::grey);
        }
    }
};
