#include "ui/NodeController.h"
#include "data/ParameterAttachment.h"
#include "ui/AnalyzerBase.h"
#include "utils/EqParam.h"
#include "utils/FilterParam.h"
#include <JuceHeader.h>

inline bool isCutFilter (ChainPositions chainPosition)
{
    return chainPosition == ChainPositions::LOWCUT || chainPosition == ChainPositions::HIGHCUT;
}

NodeController::NodeController (APVTS& apv) : apvts (apv)
{
    const int numPositions = 8;
    for (auto ch : { Channel::LEFT, Channel::RIGHT })
    {
        for (int i = 0; i < numPositions; ++i)
        {
            auto idx = static_cast<size_t> (ch) * numPositions + i;
            auto pos = static_cast<ChainPositions> (i);

            auto& node = nodes[idx];
            node = std::make_unique<AnalyzerNode> (pos, ch);
            node->addMouseListener (this, false);
            addChildComponent (node.get());
            addAndMakeVisible (node.get());

            auto& band = bands[idx];
            band = std::make_unique<AnalyzerBand> (pos, ch);
            band->addMouseListener (this, false);
            addChildComponent (band.get());

            auto& freqAttachment = freqAttachments[idx];
            auto freqName = FilterInfo::getParameterName (i, ch, FilterInfo::FilterParam::FREQUENCY);
            freqAttachment = std::make_unique<ParametersAttachment> (*apvts.getParameter (freqName), nullptr);

            auto& qualityAttachment = qualityAttachments[idx];
            auto qualityName = FilterInfo::getParameterName (i, ch, FilterInfo::FilterParam::Q);
            qualityAttachment = std::make_unique<ParametersAttachment> (*apvts.getParameter (qualityName), nullptr);

            auto& gainOrSlopeAttachment = gainSlopeAttachments[idx];
            auto gainOrSlopeName = isCutFilter (pos) ? FilterInfo::getParameterName (i, ch, FilterInfo::FilterParam::SLOPE)
                                                     : FilterInfo::getParameterName (i, ch, FilterInfo::FilterParam::GAIN);
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
            DBG ("NodeController::mouseDown: NODE");
            auto* node = std::get<WidgetType::NODE> (widgetType.component);
            constrainer.setLimits (analyzerNodeArea);

            dragger.startDraggingComponent (node, e);
            getFreqAttachment (*node)->beginGesture();
            getGainSlopeAttachment (*node)->beginGesture();

            notifyOnBandSelection (node);

            break;
        }
        case ComponentType::BAND:
        {
            DBG ("NodeController::mouseDown: BAND");
            auto* band = std::get<WidgetType::BAND> (widgetType.component);

            auto bandHalfWidth = band->getWidth() / 2;
            constrainer.setLimits (analyzerNodeArea.withX (analyzerNodeArea.getX() - bandHalfWidth)
                                       .withWidth (analyzerNodeArea.getWidth() + bandHalfWidth * 2));

            dragger.startDraggingComponent (band, e);
            getFreqAttachment (*band)->beginGesture();

            if (! isCutFilter (band->getChainPosition()))
            {
                dbLevelOnClick = getGainSlopeAttachment (*band)->getDenormalizedValue();
            }
            else
            {
                dbLevelOnClick = 0.0f;
            }
            getGainSlopeAttachment (*band)->beginGesture();
            clickPosition = e.getEventRelativeTo (this).getPosition();

            notifyOnBandSelection (band);
            break;
        }
        case ComponentType::Q:
        {
            DBG ("NodeController::mouseDown: Q");
            auto* q = std::get<WidgetType::Q> (widgetType.component);
            auto x = analyzerNodeArea.getX();
            auto w = analyzerNodeArea.getWidth();

            auto centerFreq = getFreqAttachment (*q)->getDenormalizedValue();
            auto qRange = getQualityAttachment (*q)->getParameter().getNormalisableRange().getRange();

            auto qStart = qRange.getStart();
            auto qEnd = qRange.getEnd();
            if (q == &leftQControl)
            {
                auto fLeft = leftCornerForQ (centerFreq, qStart);
                auto fLeftX1 = x + juce::mapFromLog10 (fLeft, 20.0f, 20000.0f) * w;

                fLeft = leftCornerForQ (centerFreq, qEnd);
                auto fLeftX2 = x + juce::mapFromLog10 (fLeft, 20.0f, 20000.0f) * w;

                constrainer.setLimits (analyzerNodeArea.withX (fLeftX1).withRight (fLeftX2));
            }
            else
            {
                auto fRight = rightCornerForQ (centerFreq, qStart);
                auto fRightX1 = x + juce::mapFromLog10 (fRight, 20.0f, 20000.0f) * w;

                fRight = rightCornerForQ (centerFreq, qEnd);
                auto fRightX2 = x + juce::mapFromLog10 (fRight, 20.0f, 20000.0f) * w;

                constrainer.setLimits (analyzerNodeArea.withX (fRightX1).withRight (fRightX2));
            }

            getQualityAttachment (*q)->beginGesture();
            dragger.startDraggingComponent (q, e);

            notifyOnBandSelection (q);
            break;
        }
        case ComponentType::CONTROLLER:
        {
            DBG ("NodeController::mouseDown: CONTROLLER");
            notifyOnClearSelection();
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
            DBG ("NodeController::mouseMove: NODE");
            auto* node = std::get<WidgetType::NODE> (widgetType.component);
            deselectAllNodes();
            hideAllBands();

            auto idx = getNodeIndex (*node);
            node->displayAsSelected (true);
            node->toFront (true);

            auto band = bands[idx].get();
            band->displayAsSelected (true);
            band->setVisible (true);

            leftQControl.setVisible (false);
            rightQControl.setVisible (false);

            notifyOnBandMouseOver (node);

            if (qControlActive)
            {
                if (leftQControl.getChannel() != node->getChannel() || leftQControl.getChainPosition() != node->getChainPosition())
                {
                    leftQControl.setVisible (false);
                    rightQControl.setVisible (false);
                }
            }
            break;
        }
        case ComponentType::BAND:
        {
            DBG ("NodeController::mouseMove: BAND");
            auto* band = std::get<WidgetType::BAND> (widgetType.component);
            deselectAllNodes();
            hideAllBands();

            auto idx = getNodeIndex (*band);
            band->displayAsSelected (true);
            band->setVisible (true);

            auto node = nodes[idx].get();
            node->displayAsSelected (true);
            node->toFront (true);

            leftQControl.displayAsSelected (false);
            rightQControl.displayAsSelected (false);

            notifyOnBandMouseOver (band);

            if (qControlActive)
            {
                if (leftQControl.getChannel() != band->getChannel() || leftQControl.getChainPosition() != band->getChainPosition())
                {
                    leftQControl.setVisible (false);
                    rightQControl.setVisible (false);
                }
            }

            break;
        }
        case ComponentType::Q:
        {
            DBG ("NodeController::mouseMove: Q");
            deselectAllNodes();
            hideAllBands();

            leftQControl.displayAsSelected (true);
            rightQControl.displayAsSelected (true);

            auto idx = getNodeIndex (leftQControl);
            nodes[idx]->displayAsSelected (true);

            bands[idx]->displayAsSelected (true);
            bands[idx]->setVisible (true);

            leftQControl.setVisible (true);
            rightQControl.setVisible (true);

            notifyOnBandMouseOver (&leftQControl);

            break;
        }
        case ComponentType::CONTROLLER:
        {
            DBG ("NodeController::mouseMove: CONTROLLER");
            leftQControl.setVisible (false);
            rightQControl.setVisible (false);

            for (auto n : nodeSelectionBoundingBoxes)
            {
                if (n.bounds.contains (e.getEventRelativeTo (this).getPosition()))
                {
                    jassert (n.node != nullptr);
                    bands[getNodeIndex (*n.node)]->setVisible (true);
                    n.node->displayAsSelected (true);
                    n.node->toFront (true);
                    notifyOnBandMouseOver (n.node);
                    break;
                }
            }
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
    deselectAllNodes();
    hideAllBands();

    auto widgetType = getComponentForMouseEvent (e);
    switch (widgetType.type)
    {
        case ComponentType::NODE:
        {
            DBG ("NodeController::mouseUp: NODE");
            auto* node = std::get<WidgetType::NODE> (widgetType.component);

            node->displayAsSelected (true);

            auto idx = getNodeIndex (*node);
            auto band = bands[idx].get();
            band->displayAsSelected (true);
            band->setVisible (true);

            node->toFront (true);

            getFreqAttachment (*node)->endGesture();
            getGainSlopeAttachment (*node)->endGesture();

            auto pos = node->getChainPosition();
            leftQControl.setChainPosition (pos);
            rightQControl.setChainPosition (pos);

            auto ch = node->getChannel();
            leftQControl.setChannel (ch);
            rightQControl.setChannel (ch);

            leftQControl.setBounds (band->getX(), band->getY(), 2, band->getHeight());
            rightQControl.setBounds (band->getRight() - 1, band->getY(), 2, band->getHeight());
            leftQControl.setVisible (true);
            rightQControl.setVisible (true);
            leftQControl.toFront (false);
            rightQControl.toFront (false);

            qControlActive = true;

            notifyOnBandMouseOver (node);

            break;
        }
        case ComponentType::BAND:
        {
            DBG ("NodeController::mouseUp: BAND");
            auto* band = std::get<WidgetType::BAND> (widgetType.component);
            band->displayAsSelected (true);
            band->setVisible (true);

            auto idx = getNodeIndex (*band);
            auto node = nodes[idx].get();
            node->displayAsSelected (true);
            node->toFront (true);

            getFreqAttachment (*band)->endGesture();
            getGainSlopeAttachment (*band)->endGesture();

            notifyOnBandMouseOver (band);

            break;
        }
        case ComponentType::Q:
        {
            DBG ("NodeController::mouseUp: Q");
            auto* q = std::get<WidgetType::Q> (widgetType.component);
            q->displayAsSelected (true);
            auto idx = getNodeIndex (*q);
            bands[idx]->displayAsSelected (true);
            bands[idx]->setVisible (true);

            getQualityAttachment (*q)->endGesture();

            notifyOnBandMouseOver (q);

            break;
        }
        case ComponentType::CONTROLLER:
        {
            DBG ("NodeController::mouseUp: CONTROLLER");
            notifyOnClearSelection();
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
            DBG ("NodeController::mouseDrag: NODE");
            auto* node = std::get<WidgetType::Indices::NODE> (widgetType.component);
            dragger.dragComponent (node, e, &constrainer);

            auto nodeBounds = node->getBounds();
            double normalizedX = static_cast<double> (nodeBounds.getCentreX() - fftBoundingBox.getX())
                                 / static_cast<double> (fftBoundingBox.getWidth());
            auto frequency = juce::mapToLog10 (normalizedX, 20.0, 20000.0);

            getFreqAttachment (*node)->setValueAsPartOfGesture (frequency);

            if (isCutFilter (node->getChainPosition()))
            {
                const int numSlopes = 8;
                int y = node->getBounds().getCentreY() - fftBoundingBox.getY();
                int division = fftBoundingBox.getHeight() / (numSlopes + 1);

                int index = y / division;
                getGainSlopeAttachment (*node)->setValueAsPartOfGesture (static_cast<float> (numSlopes - index));
            }
            else
            {
                auto gain = juce::jmap (static_cast<float> (node->getBounds().getCentreY()),
                                        static_cast<float> (fftBoundingBox.getBottom()),
                                        static_cast<float> (fftBoundingBox.getY()),
                                        RESPONSE_CURVE_MIN_DB,
                                        RESPONSE_CURVE_MAX_DB);

                getGainSlopeAttachment (*node)->setValueAsPartOfGesture (gain);
            }

            notifyOnBandSelection (node);

            break;
        }
        case ComponentType::BAND:
        {
            DBG ("NodeController::mouseDrag: BAND");
            /* auto* band = std::get<WidgetType::BAND> (widgetType.component); */
            /* dragger.dragComponent (band, e, &constrainer); */
            break;
        }
        case ComponentType::Q:
        {
            DBG ("NodeController::mouseDrag: Q");
            /* auto* q = std::get<WidgetType::Q> (widgetType.component); */
            /* dragger.dragComponent (q, e, &constrainer); */
            break;
        }
        case ComponentType::CONTROLLER:
        {
            DBG ("NodeController::mouseDrag: Controller");
            /* auto* controller = std::get<WidgetType::CONTROLLER> (widgetType.component); */
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
            DBG ("NodeController::mouseEnter: NODE");
            auto* node = std::get<WidgetType::Indices::NODE> (widgetType.component);
            deselectAllNodes();
            hideAllBands();
            node->displayAsSelected (true);

            auto idx = getNodeIndex (*node);
            bands[idx]->setVisible (true);
            bands[idx]->displayAsSelected (true);
            node->toFront (true);
            if (qControlActive)
            {
                leftQControl.setVisible (true);
                rightQControl.setVisible (true);
            }
            notifyOnBandMouseOver (node);

            break;
        }
        case ComponentType::BAND:
        {
            DBG ("NodeController::mouseEnter: BAND");
            auto* band = std::get<WidgetType::BAND> (widgetType.component);
            band->displayAsSelected (true);
            if (! qControlActive)
            {
                band->toFront (false);
            }
            notifyOnBandMouseOver (band);
            break;
        }
        case ComponentType::Q:
        {
            DBG ("NodeController::mouseEnter: Q");
            auto* q = std::get<WidgetType::Q> (widgetType.component);
            q->displayAsSelected (true);
            q->toFront (false);
            notifyOnBandMouseOver (q);
            break;
        }
        case ComponentType::CONTROLLER:
        {
            DBG ("NodeController::mouseEnter: CONTROLLER");
            deselectAllNodes();
            hideAllBands();
            notifyOnClearSelection();
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
    /* auto widgetType = getComponentForMouseEvent (e); */
    /* switch (widgetType.type) */
    /* { */
    /*     case ComponentType::NODE: */
    /*     { */
    /*         auto* node = std::get<WidgetType::Indices::NODE> (widgetType.component); */
    /**/
    /*         break; */
    /*     } */
    /*     case ComponentType::BAND: */
    /*     { */
    /*         auto* band = std::get<WidgetType::BAND> (widgetType.component); */
    /*         break; */
    /*     } */
    /*     case ComponentType::Q: */
    /*     { */
    /*         auto* q = std::get<WidgetType::Q> (widgetType.component); */
    /*         break; */
    /*     } */
    /*     case ComponentType::CONTROLLER: */
    /*     { */
    /*         auto* controller = std::get<WidgetType::CONTROLLER> (widgetType.component); */
    /*         break; */
    /*     } */
    /*     case ComponentType::INVALID: */
    /*     case ComponentType::NUM_TYPES: */
    /*     default: */
    /*         jassertfalse; */
    /* } */
}

void NodeController::mouseDoubleClick (const juce::MouseEvent& e)
{
    /* auto widgetType = getComponentForMouseEvent (e); */
    /* switch (widgetType.type) */
    /* { */
    /*     case ComponentType::NODE: */
    /*     { */
    /*         auto* node = std::get<WidgetType::Indices::NODE> (widgetType.component); */
    /**/
    /*         break; */
    /*     } */
    /*     case ComponentType::BAND: */
    /*     { */
    /*         auto* band = std::get<WidgetType::BAND> (widgetType.component); */
    /*         break; */
    /*     } */
    /*     case ComponentType::Q: */
    /*     { */
    /*         auto* q = std::get<WidgetType::Q> (widgetType.component); */
    /*         break; */
    /*     } */
    /*     case ComponentType::CONTROLLER: */
    /*     { */
    /*         auto* controller = std::get<WidgetType::CONTROLLER> (widgetType.component); */
    /*         break; */
    /*     } */
    /*     case ComponentType::INVALID: */
    /*     case ComponentType::NUM_TYPES: */
    /*     default: */
    /*         jassertfalse; */
    /* } */
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
    AnalyzerBase::resized();
    createAnalyzerNodeArea();
    refreshWidgets();
}

float NodeController::leftCornerForQ (float freq, float q)
{
    // lower cutoff frequency for the bandpass filter represented by the peak filter
    return freq * (std::sqrt (1.0f + 1.0f / (4.0f * q * q)) - 1.0f / (2.0f * q));
}

float NodeController::rightCornerForQ (float freq, float q)
{
    // upper cutoff frequency for the bandpass filter represented by the peak filter
    return freq * (std::sqrt (1.0f + 1.0f / (4.0f * q * q)) + 1.0f / (2.0f * q));
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
    return node.getChannel() == Channel::LEFT ? static_cast<size_t> (node.getChainPosition())
                                              : static_cast<size_t> (node.getChainPosition()) + 8;
}

ParametersAttachment* NodeController::getFreqAttachment (AnalyzerWidgetBase& node)
{
    int chIndex = static_cast<int> (node.getChannel()) * 8;
    ChainPositions cp = node.getChainPosition();
    size_t filterNum = static_cast<size_t> (cp);

    return freqAttachments[filterNum + chIndex].get();
}

ParametersAttachment* NodeController::getQualityAttachment (AnalyzerWidgetBase& node)
{
    int chIndex = static_cast<int> (node.getChannel()) * 8;
    ChainPositions cp = node.getChainPosition();
    size_t filterNum = static_cast<size_t> (cp);

    return qualityAttachments[filterNum + chIndex].get();
}

ParametersAttachment* NodeController::getGainSlopeAttachment (AnalyzerWidgetBase& node)
{
    int chIndex = static_cast<int> (node.getChannel()) * 8;
    ChainPositions cp = node.getChainPosition();
    size_t filterNum = static_cast<size_t> (cp);

    return gainSlopeAttachments[filterNum + chIndex].get();
}

void NodeController::repositionNodes()
{
    for (auto& node : nodes)
    {
        auto* freqAttachment = getFreqAttachment (*node);
        auto freq = freqAttachment->getDenormalizedValue();

        auto* gainSlopeAttachment = getGainSlopeAttachment (*node);
        auto gainOrSlope = gainSlopeAttachment->getDenormalizedValue();

        if (! nodeNeedsUpdate (*node, freq, gainOrSlope))
            continue;

        repositionNode (*node, freq, gainOrSlope);
    }
}

void NodeController::repositionNode (AnalyzerNode& node, float freq, float /*gainOrSlope*/)
{
    int x = fftBoundingBox.getX() + juce::mapFromLog10 (freq, 20.0f, 20000.0f) * fftBoundingBox.getWidth();
    int y = 0;
    auto* param = getGainSlopeAttachment (node);
    auto value = param->getDenormalizedValue();
    if (isCutFilter (node.getChainPosition()))
    {
        int numSlopes = param->getParameter().getNumSteps();
        y = juce::jmap ((int) value, 0, numSlopes - 1, analyzerNodeArea.getBottom(), analyzerNodeArea.getY());
    }
    else
    {
        y = juce::jmap (value,
                        RESPONSE_CURVE_MIN_DB,
                        RESPONSE_CURVE_MAX_DB,
                        static_cast<float> (fftBoundingBox.getBottom()),
                        static_cast<float> (fftBoundingBox.getY()));
    }
    node.setCentrePosition (juce::Point<int> (x, y));
}

void NodeController::repositionBands()
{
    juce::Rectangle<int> bounds;
    auto w = analyzerNodeArea.getWidth();
    auto b = analyzerNodeArea.getBottom();
    auto x = analyzerNodeArea.getX();
    auto y = analyzerNodeArea.getY();

    for (auto& band : bands)
    {
        auto f = getFreqAttachment (*band)->getDenormalizedValue();
        auto q = getQualityAttachment (*band)->getDenormalizedValue();

        auto fLeft = leftCornerForQ (f, q);
        auto fLeftX = x + juce::mapFromLog10 (fLeft, 20.0f, 20000.0f) * w;

        auto fRight = rightCornerForQ (f, q);
        auto fRightX = x + juce::mapFromLog10 (fRight, 20.0f, 20000.0f) * w;

        bounds.setX (fLeftX);
        bounds.setY (y);
        bounds.setRight (fRightX);
        bounds.setBottom (b);

        band->setBounds (bounds);
    }
}

void NodeController::repositionQControls()
{
    auto idx = getNodeIndex (leftQControl);
    auto* band = bands[idx].get();

    leftQControl.setBounds (band->getX() - 1, band->getY(), 2, band->getHeight());
    rightQControl.setBounds (band->getRight() - 1, band->getY(), 2, band->getHeight());
}

bool NodeController::nodeNeedsUpdate (AnalyzerNode& node, float freq, float gainOrSlope)
{
    return node.updateFrequency (freq) || node.updateGainOrSlope (gainOrSlope);
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
    for (auto& n : nodes)
    {
        n->toFront (true);
    }
    leftQControl.toFront (true);
    rightQControl.toFront (true);
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
    analyzerNodeArea = fftBoundingBox;

    auto controlRange = getGainSlopeAttachment (*nodes[3])->getParameter().getNormalisableRange().getRange();
    auto pixelsPerDB = fftBoundingBox.getHeight() / (RESPONSE_CURVE_MAX_DB - RESPONSE_CURVE_MIN_DB);
    analyzerNodeArea.setTop (fftBoundingBox.getY() + pixelsPerDB * (RESPONSE_CURVE_MAX_DB - controlRange.getEnd()));
    analyzerNodeArea.setBottom (fftBoundingBox.getBottom() - pixelsPerDB * (controlRange.getStart() - RESPONSE_CURVE_MIN_DB));
}

void NodeController::updateNodesVisibility()
{
    auto eqMode = static_cast<EqMode> (apvts.getRawParameterValue ("eq_mode")->load());
    auto numNodes = eqMode == EqMode::STEREO ? nodes.size() / 2 : nodes.size();

    updateVisibilities (nodes, numNodes);
}

void NodeController::rebuildNodeSelectionBoundingBox()
{
    for (auto& n : nodeSelectionBoundingBoxes)
    {
        n.node = nullptr;
        n.bounds = {};
    }

    std::vector<AnalyzerNode*> nodesToDisplay;
    for (auto& node : nodes)
    {
        if (node->isVisible())
            nodesToDisplay.push_back (node.get());
    }

    std::sort (nodesToDisplay.begin(),
               nodesToDisplay.end(),
               [] (const auto& a, const auto& b) { return a->getBounds().getCentreX() < b->getBounds().getCentreX(); });

    juce::Rectangle<int> rect, previous;
    for (size_t i = 0; i < nodesToDisplay.size(); ++i)
    {
        rect.setY (fftBoundingBox.getY());
        rect.setBottom (fftBoundingBox.getBottom());
        rect.setX (i == 0 ? fftBoundingBox.getX() : previous.getRight());

        auto* node = nodesToDisplay[i];
        nodeSelectionBoundingBoxes[i].node = node;

        if (i < nodesToDisplay.size() - 1)
        {
            auto next = nodesToDisplay[i + 1];
            auto nodeCentreX = node->getBounds().getCentreX();
            rect.setRight (nodeCentreX + ((next->getBounds().getCentreX() - nodeCentreX) / 2.0f));
        }
        else
        {
            rect.setRight (fftBoundingBox.getRight());
        }

        previous = rect;
        nodeSelectionBoundingBoxes[i].bounds = rect;
    }
}
