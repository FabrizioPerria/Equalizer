#pragma once

#include "data/ParameterAttachment.h"
#include "ui/AnalyzerBase.h"
#include "ui/AnalyzerWidgets.h"
#include "utils/AllParamsListener.h"
#include "utils/ChainHelpers.h"
#include <JuceHeader.h>
#include <tuple>

struct CustomConstrainer : juce::ComponentBoundsConstrainer
{
    void setLimits (const juce::Rectangle<int>& limits)
    {
        boundsLimit = limits;
    }

    juce::Rectangle<int> boundsLimit;
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

    juce::Rectangle<int> analyzerNodeArea;

    juce::ListenerList<NodeListener> listeners;

    juce::Point<int> clickPosition;
    float dbLevelOnClick { 0.f };

    bool qControlActive { false };

    enum class ComponentType
    {
        NODE,
        BAND,
        Q,
        CONTROLLER,
        INVALID,
        NUM_TYPES
    };

    struct WidgetType
    {
        ComponentType type;
        std::tuple<AnalyzerNode*, AnalyzerBand*, AnalyzerQControl*, NodeController*> component;

        enum Indices
        {
            NODE,
            BAND,
            Q,
            CONTROLLER
        };
    };

    ChainPositions chainPosition;
    Channel channel;

    WidgetType getComponentForMouseEvent (const juce::MouseEvent& e);
    size_t getNodeIndex (AnalyzerWidgetBase& node);

    ParametersAttachment* getFreqAttachment (AnalyzerWidgetBase& node);
    ParametersAttachment* getQualityAttachment (AnalyzerWidgetBase& node);
    ParametersAttachment* getGainSlopeAttachment (AnalyzerWidgetBase& node);

    void repositionNodes();
    void repositionNode (AnalyzerWidgetBase& node, float freq, float gainOrSlope);
    void repositionBands();

    void repositionQControls();

    bool nodeNeedsUpdate (AnalyzerWidgetBase& node, float freq, float gainOrSlope);

    void notifyOnBandSelection (AnalyzerWidgetBase* widget);
    void notifyOnBandMouseOver (AnalyzerWidgetBase* widget);
    void notifyOnClearSelection();

    void reorderWidgets();
    void refreshWidgets();
    void hideAllBands();
    void deselectAllNodes();

    void createAnalyzerNodeArea();

    void updateNodesVisibility();

    void rebuildNodeSelectionBoundingBox();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NodeController)
};
