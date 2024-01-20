#pragma once

#include "data/ParameterAttachment.h"
#include "ui/AnalyzerBase.h"
#include "ui/AnalyzerWidgets.h"
#include "utils/AllParamsListener.h"
#include "utils/ChainHelpers.h"
#include <JuceHeader.h>

struct CustomConstrainer : juce::Rectangle<int>
{
    CustomConstrainer (juce::Rectangle<int> bounds) : juce::RectangleConstrainer (bounds)
    {
    }

    juce::Rectangle<int> getBoundsForRectangle (const juce::Rectangle<int>& originalBounds,
                                                const juce::Rectangle<int>& /*proposedBounds*/,
                                                bool /*isStretchingTop*/,
                                                bool /*isStretchingLeft*/,
                                                bool /*isStretchingBottom*/,
                                                bool /*isStretchingRight*/) override
    {
        return originalBounds;
    }
};
struct NodeController : AnalyzerBase
{
    using APVTS = juce::AudioProcessorValueTreeState;
    NodeController (APVTS& apv);

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;

    struct NodeListener
    {
        virtual ~NodeListener() = default;
        virtual void bandMouseOver (ChainPositions, Channel) = 0;
        virtual void bandSelected (ChainPositions, Channel) = 0;
        virtual void clearSelection() = 0;
    };

    void addListener (NodeListener* listener);
    void removeListener (NodeListener* listener);

    void resized() override;

private:
    APVTS& apvts;

    std::array<std::unique_ptr<AnalyzerNode>, 16> nodes;
    std::array<std::unique_ptr<AnalyzerBand>, 16> bands;

    AnalyzerQControl leftQControl, rightQControl;

    std::array<std::unique_ptr<ParametersAttachment>, 16> freqAttachments;
    std::array<std::unique_ptr<ParametersAttachment>, 16> qualityAttachments;
    std::array<std::unique_ptr<ParametersAttachment>, 16> gainSlopeAttachments;

    juce::ComponentDragger dragger;
    CustomConstrainer constrainer;

    std::unique_ptr<AllParamsListener> allParamsListener;

    juce::ListenerList<NodeListener> listeners;

    AnalyzerWidgetBase getComponentForMouseEvent (const juce::MouseEvent& e);
    void notifyOnBandSelection (AnalyzerWidgetBase* widget);
    void notifyOnBandMouseOver (AnalyzerWidgetBase* widget);
    void notifyOnClearSelection();

    void refreshWidgets();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NodeController)
};
