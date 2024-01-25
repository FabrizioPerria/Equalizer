#pragma once

#include "data/ParameterAttachment.h"
#include "ui/AnalyzerBase.h"
#include "ui/AnalyzerWidgets.h"
#include "utils/AllParamsListener.h"
#include "utils/ChainHelpers.h"
#include "utils/FilterParam.h"
#include <JuceHeader.h>
#include <tuple>

struct CustomConstrainer : juce::ComponentBoundsConstrainer
{
    void setLimits (const juce::Rectangle<int>& limits)
    {
        boundsLimit = limits;
    }

    void checkBounds (juce::Rectangle<int>& bounds,
                      const juce::Rectangle<int>& /* old */,
                      const juce::Rectangle<int>& /* limits */,
                      bool /* isStretchingTop */,
                      bool /* isStretchingLeft */,
                      bool /* isStretchingBottom */,
                      bool /* isStretchingRight */) override
    {
        auto centre = bounds.getCentre();

        if (centre.getY() < boundsLimit.getY())
            centre.setY (boundsLimit.getY());
        if (centre.getY() > boundsLimit.getBottom())
            centre.setY (boundsLimit.getBottom());
        if (centre.getX() > boundsLimit.getRight())
            centre.setX (boundsLimit.getRight());
        if (centre.getX() < boundsLimit.getX())
            centre.setX (boundsLimit.getX());

        bounds.setCentre (centre);
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

    float leftCornerForQ (float freq, float q);
    float rightCornerForQ (float freq, float q);

    WidgetType getComponentForMouseEvent (const juce::MouseEvent& e);
    size_t getNodeIndex (AnalyzerWidgetBase& node);

    ParametersAttachment* getFreqAttachment (AnalyzerWidgetBase& node);
    ParametersAttachment* getQualityAttachment (AnalyzerWidgetBase& node);
    ParametersAttachment* getGainSlopeAttachment (AnalyzerWidgetBase& node);

    struct NodeWithBounds
    {
        AnalyzerWidgetBase* node = nullptr;
        juce::Rectangle<int> bounds = {};
    };

    std::array<NodeWithBounds, 16> nodeSelectionBoundingBoxes;

    void repositionNodes();
    void repositionNode (AnalyzerNode& node, float freq, float gainOrSlope);
    void repositionBands();

    void repositionQControls();

    bool nodeNeedsUpdate (AnalyzerNode& node, float freq, float gainOrSlope);

    void notifyOnBandSelection (AnalyzerWidgetBase* widget);
    void notifyOnBandMouseOver (AnalyzerWidgetBase* widget);
    void notifyOnClearSelection();

    void reorderWidgets();
    void refreshWidgets();
    void hideAllBands();
    void deselectAllNodes();

    void createAnalyzerNodeArea();

    void updateNodesVisibility();

    template <typename WidgetType>
    void updateVisibilities (WidgetType& container, int num)
    {
        for (auto& widget : container)
            widget->setVisible (false);

        for (size_t i = 0; i < num; ++i)
        {
            auto* widget = static_cast<AnalyzerWidgetBase*> (container[i].get());
            widget->setVisible (false);
            auto bypassParam = apvts.getParameter (FilterInfo::getParameterName (static_cast<int> (widget->getChainPosition()),
                                                                                 widget->getChannel(),
                                                                                 FilterInfo::FilterParam::BYPASS));
            widget->setVisible (bypassParam->getValue() < 0.5f);
        }
    }

    void rebuildNodeSelectionBoundingBox();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NodeController)
};
