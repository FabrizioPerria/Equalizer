#include "ui/NodeController.h"
#include "data/ParameterAttachment.h"
#include "ui/AnalyzerBase.h"
#include <JuceHeader.h>

inline bool isCutFilter (ChainPositions chainPosition)
{
    return chainPosition == ChainPositions::LOWCUT || chainPosition == ChainPositions::HIGHCUT;
}

NodeController::NodeController (APVTS& apv) : apvts (apv)
{
    const int numPositions = 8;
    for (auto channel : { Channel::LEFT, Channel::RIGHT })
    {
        for (int i = 0; i < numPositions; ++i)
        {
            auto idx = static_cast<size_t>(channel) * numPositions + i;
            auto pos = static_cast<ChainPositions> (i);
            
            auto& node = nodes[idx];
            node = std::make_unique<AnalyzerNode> (pos, channel);
            node->addMouseListener (this, false);
            addChildComponent (node.get());

            auto& band = bands[idx];
            band = std::make_unique<AnalyzerBand> (pos, channel);
            band->addMouseListener (this, false);
            addChildComponent (band.get());

            auto& freqAttachment = freqAttachments[idx];
            auto freqName = FilterInfo::getParameterName (i, channel, FilterInfo::FilterParam::FREQUENCY);
            freqAttachment = std::make_unique<ParametersAttachment> (*apvts.getParameter (freqName), nullptr);

            auto& qualityAttachment = qualityAttachments[idx];
            auto qualityName = FilterInfo::getParameterName (i, channel, FilterInfo::FilterParam::Q);
            qualityAttachment = std::make_unique<ParametersAttachment> (*apvts.getParameter (qualityName), nullptr);

            auto& gainOrSlopeAttachment = gainSlopeAttachments[idx];
            auto gainOrSlopeName = isCutFilter (pos) ? FilterInfo::getParameterName (i, channel, FilterInfo::FilterParam::SLOPE)
                                                               : FilterInfo::getParameterName (i, channel, FilterInfo::FilterParam::GAIN);
            gainOrSlopeAttachment = std::make_unique<ParametersAttachment> (*apvts.getParameter (gainOrSlopeName), nullptr);
        }
    }

    addChildComponent (leftQControl);
    leftQControl.addMouseListener (this, false);

    addChildComponent (rightQControl);
    rightQControl.addMouseListener (this, false);

    SafePointer<NodeController> safeThis (this);
    allParamsListener = std::make_unique<AllParamsListener> (&apvts,
                                                             [safeThis]()
                                                             {
                                                                 if (auto* comp = safeThis.getComponent())
                                                                     comp->refreshWidgets();
                                                             });
}

void NodeController::mouseDown (const juce::MouseEvent& e)
{
    auto widgetType = getComponentForMouseEvent (e);

    switch (widgetType.type)
    {
        case ComponentType::NODE:
        {
            auto* node = std::get<WidgetType::NODE> (widgetType.component);
            // TODO: implement
            /* node: start drag, start gesture, notify listeners [DONE] */

            /* constrainer.setLimits (analyzerNodeArea); */
            /**/
            /* dragger.startDraggingComponent (node, e); */
            /* getFreqAttachment (*node)->beginGesture(); */
            /* getGainSlopeAttachment (*node)->beginGesture(); */
            /**/
            /* notifyOnBandSelection (node); */

            break;
        }
        case ComponentType::BAND:
        {
            auto* band = std::get<WidgetType::BAND> (widgetType.component);
            //TODO: implement
            /* band: start drag, start gesture, notify listeners [DONE] */
            break;
        }
        case ComponentType::Q:
        {
            auto* q = std::get<WidgetType::Q> (widgetType.component);
            //TODO: implement
            /* Q: start drag, start gesture, notify listeners [DONE] */
            break;
        }
        case ComponentType::CONTROLLER:
        {
            auto* controller = std::get<WidgetType::CONTROLLER> (widgetType.component);
            //TODO: implement
            /* controller: do nothing, notify listeners [DONE] */
            break;
        }
        case ComponentType::INVALID:
        case ComponentType::NUM_TYPES:
        default:
            jassertfalse;
    }
}

void NodeController::mouseMove (const juce::MouseEvent& e)
{
    auto widgetType = getComponentForMouseEvent (e);

    switch (widgetType.type)
    {
        case ComponentType::NODE:
        {
            auto* node = std::get<WidgetType::NODE> (widgetType.component);
            // TODO: implement
            /* inactive Q controls: */
            /* node: highlight node, highlight band, notify listeners [DONE] */
            /* active Q controls: */
            /* node: highlight node, highlight band, notify listeners.  same a Q ? do nothing : hide Q controls [DONE] */
            break;
        }
        case ComponentType::BAND:
        {
            auto* band = std::get<WidgetType::BAND> (widgetType.component);
            //TODO: implement
            /* inactive Q controls: */
            /* band: highlight node, highlight band, notify listeners [DONE] */
            /* active Q controls: */
            /* band: highlight node, highlight band, notify listeners. same a Q ? do nothing : hide Q controls [DONE] */
            break;
        }
        case ComponentType::Q:
        {
            auto* q = std::get<WidgetType::Q> (widgetType.component);
            //TODO: implement
            /* active Q controls: */
            /* Q: highlight node, highlight band, highlight Q, notify listeners. [DONE] */
            break;
        }
        case ComponentType::CONTROLLER:
        {
            auto* controller = std::get<WidgetType::CONTROLLER> (widgetType.component);
            //TODO: implement
            /* inactive Q controls: */
            /* controller: display band under cursor, notify listeners [DONE] */
            /* active Q controls: */
            /* controller: hide Q controls, highlight band/node inside mouseOverbounds, notify listeners [ DONE] */
            break;
        }
        case ComponentType::INVALID:
        case ComponentType::NUM_TYPES:
        default:
            jassertfalse;
            break;
    }
}

void NodeController::mouseUp (const juce::MouseEvent& e)
{
    auto widgetType = getComponentForMouseEvent (e);
    switch (widgetType.type)
    {
        case ComponentType::NODE:
        {
            auto* node = std::get<WidgetType::NODE> (widgetType.component);
            // TODO: implement
            /* inactive Q controls: */
            /* node: highlight node, highlight band, end gesture, display Q controls, notify listeners [DONE] */
            /* active Q controls: */
            /* node: highlight node, highlight band, end gesture, notify listeners [DONE] */
            break;
        }
        case ComponentType::BAND:
        {
            auto* band = std::get<WidgetType::BAND> (widgetType.component);
            //TODO: implement
            /* inactive Q controls: */
            /* band: highlight band, end gesture, notify listeners [DONE] */
            /* active Q controls: */
            /* band: highlight band, end gesture, notify listeners [DONE] */
            break;
        }
        case ComponentType::Q:
        {
            auto* q = std::get<WidgetType::Q> (widgetType.component);
            //TODO: implement
            /* active Q controls: */
            /* Q: highlight Q, end gesture, notify listeners [DONE] */
            break;
        }
        case ComponentType::CONTROLLER:
        {
            auto* controller = std::get<WidgetType::CONTROLLER> (widgetType.component);
            //TODO: implement
            /* inactive Q controls: */
            /* controller: do nothing, notify listeners [DONE] */
            /* active Q controls: */
            /* controller: notify listeners [DONE] */
            break;
        }
        case ComponentType::INVALID:
        case ComponentType::NUM_TYPES:
        default:
            jassertfalse;
            break;
    }
}

void NodeController::mouseDrag (const juce::MouseEvent& e)
{
    auto widgetType = getComponentForMouseEvent (e);
    switch (widgetType.type)
    {
        case ComponentType::NODE:
        {
            auto* node = std::get<WidgetType::Indices::NODE> (widgetType.component);
            /* dragger.dragComponent (node, e, &constrainer); */
            /**/
            /* double normalizedX = static_cast<double> ((node->getBounds().getCentreX() - fftBoundingBox.getX()) / fftBoundingBox.getWidth()); */
            /* auto frequency = juce::mapToLog10 (normalizedX, 20.0, 20000.0); */
            /* getFreqAttachment (*node)->setValueAsPartOfGesture (frequency); */
            /**/
            /* if (isCutFilter (node->getChainPosition())) */
            /* { */
            /*     const int numSlopes = 8; */
            /*     int y = node->getBounds().getCentreY() - fftBoundingBox.getY(); */
            /*     int division = fftBoundingBox.getHeight() / (numSlopes + 1); */
            /**/
            /*     int index = y / division; */
            /*     getGainSlopeAttachment (*node)->setValueAsPartOfGesture (static_cast<float> (numSlopes - index)); */
            /* } */
            /* else */
            /* { */
            /*     auto gain = juce::jmap (static_cast<float> (node->getBounds().getCentreY()), */
            /*                             static_cast<float> (fftBoundingBox.getBottom()), */
            /*                             static_cast<float> (fftBoundingBox.getY()), */
            /*                             RESPONSE_CURVE_MIN_DB, */
            /*                             RESPONSE_CURVE_MAX_DB); */
            /**/
            /*     gain = juce::jlimit (RESPONSE_CURVE_MIN_DB, RESPONSE_CURVE_MAX_DB, gain); // TODO: are these right?? */
            /*     getGainSlopeAttachment (*node)->setValueAsPartOfGesture (gain); */
            /* } */
            /**/
            /* notifyOnBandSelection (node); */

            break;
        }
        case ComponentType::BAND:
        {
            auto* band = std::get<WidgetType::BAND> (widgetType.component);
            dragger.dragComponent (band, e, &constrainer);
            //TODO: implement
            /* inactive Q controls: */
            /* band: drag band, update parameter, notify listeners [DONE] */
            /* active Q controls: */
            /* band: drag band, update parameter, notify listeners [DONE] */
            break;
        }
        case ComponentType::Q:
        {
            auto* q = std::get<WidgetType::Q> (widgetType.component);
            dragger.dragComponent (q, e, &constrainer);
            //TODO: implement
            /* active Q controls: */
            /* Q: drag Q, update parameter, notify listeners [DONE] */
            break;
        }
        case ComponentType::CONTROLLER:
        {
            auto* controller = std::get<WidgetType::CONTROLLER> (widgetType.component);
            //TODO: implement
            /* inactive Q controls: */
            /* controller: do nothing, notify listeners [DONE] */
            /* active Q controls: */
            /* controller: do nothing, notify listeners [DONE] */
            break;
        }
        case ComponentType::INVALID:
        case ComponentType::NUM_TYPES:
        default:
            jassertfalse;
    }
}

void NodeController::mouseEnter (const juce::MouseEvent& e)
{
    auto widgetType = getComponentForMouseEvent (e);
    switch (widgetType.type)
    {
        case ComponentType::NODE:
        {
            auto* node = std::get<WidgetType::Indices::NODE> (widgetType.component);
            //TODO: implement
            /* inactive Q controls: */
            /* node: highlight node, highlight band, notify listeners [DONE] */
            /* active Q controls: */
            /* node: highlight node, highlight band, notify listeners [DONE] */

            break;
        }
        case ComponentType::BAND:
        {
            auto* band = std::get<WidgetType::BAND> (widgetType.component);
            //TODO: implement
            /* inactive Q controls: */
            /* band: highlight node, highlight band, notify listeners [DONE] */
            /* active Q controls: */
            /* band: highlight node, highlight band, notify listeners [DONE] */
            break;
        }
        case ComponentType::Q:
        {
            auto* q = std::get<WidgetType::Q> (widgetType.component);
            //TODO: implement
            /* active Q controls: */
            /* Q: change cursor, highlight node, highlight band, notify listeners [DONE] */
            break;
        }
        case ComponentType::CONTROLLER:
        {
            auto* controller = std::get<WidgetType::CONTROLLER> (widgetType.component);
            //TODO: implement
            /* inactive Q controls: */
            /* controller: un-highlight everything, notify listeners [DONE] */
            /* active Q controls: */
            /* controller: un-highlight everything, notify listeners [DONE] */
            break;
        }
        case ComponentType::INVALID:
        case ComponentType::NUM_TYPES:
        default:
            jassertfalse;
    }
}

void NodeController::mouseExit (const juce::MouseEvent& e)
{
    auto widgetType = getComponentForMouseEvent (e);
    switch (widgetType.type)
    {
        case ComponentType::NODE:
        {
            auto* node = std::get<WidgetType::Indices::NODE> (widgetType.component);
            //TODO: implement
            /* inactive Q controls: */
            /* node: un-highlight node, notify listeners */
            /* active Q controls: */
            /* node: un-highlight node, notify listeners */

            break;
        }
        case ComponentType::BAND:
        {
            auto* band = std::get<WidgetType::BAND> (widgetType.component);
            //TODO: implement
            /* inactive Q controls: */
            /* band: un-highlight band, notify listeners */
            /* active Q controls: */
            /* band: un-highlight band, notify listeners */
            break;
        }
        case ComponentType::Q:
        {
            auto* q = std::get<WidgetType::Q> (widgetType.component);
            //TODO: implement
            /* active Q controls: */
            /* Q: reset cursor, un-highlight Q, notify listeners */
            break;
        }
        case ComponentType::CONTROLLER:
        {
            auto* controller = std::get<WidgetType::CONTROLLER> (widgetType.component);
            //TODO: implement
            /* inactive Q controls: */
            /* controller: un-highlight everything, notify listeners [DONE] */
            /* active Q controls: */
            /* controller: un-highlight everything, notify listeners [DONE] */
            break;
        }
        case ComponentType::INVALID:
        case ComponentType::NUM_TYPES:
        default:
            jassertfalse;
    }
}

void NodeController::mouseDoubleClick (const juce::MouseEvent& e)
{
    auto widgetType = getComponentForMouseEvent (e);
    switch (widgetType.type)
    {
        case ComponentType::NODE:
        {
            auto* node = std::get<WidgetType::Indices::NODE> (widgetType.component);
            //TODO: implement
            /* inactive Q controls: */
            /* node: reset freq/gain/slope */
            /* active Q controls: */
            /* node: reset freq/gain/slope */

            break;
        }
        case ComponentType::BAND:
        {
            auto* band = std::get<WidgetType::BAND> (widgetType.component);
            //TODO: implement
            /* inactive Q controls: */
            /* band: do nothing */
            /* active Q controls: */
            /* band: do nothing */
            break;
        }
        case ComponentType::Q:
        {
            auto* q = std::get<WidgetType::Q> (widgetType.component);
            //TODO: implement
            /* active Q controls: */
            /* Q: reset Q */
            break;
        }
        case ComponentType::CONTROLLER:
        {
            auto* controller = std::get<WidgetType::CONTROLLER> (widgetType.component);
            //TODO: implement
            /* inactive Q controls: */
            /* controller: do nothing */
            /* active Q controls: */
            /* controller: do nothing */
            break;
        }
        case ComponentType::INVALID:
        case ComponentType::NUM_TYPES:
        default:
            jassertfalse;
    }
}

void NodeController::addListener (NodeListener* listener)
{
    listeners.add (listener);
}

void NodeController::removeListener (NodeListener* listener)
{
    listeners.remove (listener);
}

void NodeController::resized()
{
}

NodeController::WidgetType NodeController::getComponentForMouseEvent (const juce::MouseEvent& e)
{
    if (auto* node = dynamic_cast<AnalyzerNode*> (e.eventComponent))
        return { ComponentType::NODE, { node, nullptr, nullptr, nullptr } };

    if (auto* band = dynamic_cast<AnalyzerBand*> (e.eventComponent))
        return { ComponentType::BAND, { nullptr, band, nullptr, nullptr } };

    if (auto* q = dynamic_cast<AnalyzerQControl*> (e.eventComponent))
        return { ComponentType::Q, { nullptr, nullptr, q, nullptr } };

    if (auto* controller = dynamic_cast<NodeController*> (e.eventComponent))
        return { ComponentType::CONTROLLER, { nullptr, nullptr, nullptr, controller } };

    jassertfalse;
    return { ComponentType::INVALID, { nullptr, nullptr, nullptr, nullptr } };
}

size_t NodeController::getNodeIndex (AnalyzerWidgetBase& node)
{
    return 0;
}

ParametersAttachment* NodeController::getFreqAttachment (AnalyzerWidgetBase& node)
{
    return nullptr;
}

ParametersAttachment* NodeController::getQualityAttachment (AnalyzerWidgetBase& node)
{
    return nullptr;

}

ParametersAttachment* NodeController::getGainSlopeAttachment (AnalyzerWidgetBase& node)
{
    return nullptr;

}

void NodeController::repositionNodes()
{
}

void NodeController::repositionNode (AnalyzerWidgetBase& node, float freq, float gainOrSlope)
{
}

void NodeController::repositionBands()
{
}

void NodeController::repositionQControls()
{
}

bool NodeController::nodeNeedsUpdate (AnalyzerWidgetBase& node, float freq, float gainOrSlope)
{
    return false;
}

void NodeController::notifyOnBandSelection (AnalyzerWidgetBase* widget)
{
    listeners.call ([widget] (NodeListener& l) { //
        l.bandSelected (widget->getChainPosition(), widget->getChannel());
    });
}

void NodeController::notifyOnBandMouseOver (AnalyzerWidgetBase* widget)
{
    listeners.call ([widget] (NodeListener& l) { //
        l.bandMouseOver (widget->getChainPosition(), widget->getChannel());
    });
}

void NodeController::notifyOnClearSelection()
{
    listeners.call ([] (NodeListener& l) { //
        l.clearSelection();
    });
}

void NodeController::reorderWidgets()
{
}

void NodeController::refreshWidgets()
{
    repositionNodes();
    updateNodesVisibility();
    repositionBands();
    rebuildNodeSelectionBoundingBox();
    repositionQControls();
    reorderWidgets();
}

void NodeController::hideAllBands()
{
    for (auto& band : bands)
    {
        band->setVisible (false);
        band->displayAsSelected (false);
    }
}

void NodeController::deselectAllNodes()
{
    for (auto& node : nodes)
    {
        node->displayAsSelected (false);
    }
}

void NodeController::createAnalyzerNodeArea()
{
}

void NodeController::updateNodesVisibility()
{
}

void NodeController::rebuildNodeSelectionBoundingBox()
{
}
