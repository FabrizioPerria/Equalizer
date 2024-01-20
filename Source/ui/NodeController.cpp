#include "ui/NodeController.h"
#include "data/ParameterAttachment.h"
#include <JuceHeader.h>

bool shouldGetSlope (ChainPositions chainPosition)
{
    return chainPosition == ChainPositions::LOWCUT || chainPosition == ChainPositions::HIGHCUT;
}

NodeController::NodeController (APVTS& apv) : apvts (apv)
{
    for (auto channel : { Channel::LEFT, Channel::RIGHT })
    {
        int i = 0;
        for (auto& node : nodes)
        {
            auto chainPosition = static_cast<ChainPositions> (i);
            node = std::make_unique<AnalyzerNode> (chainPosition, channel);
            addMouseListener (this, false);
            addChildComponent (node.get());

            auto& band = bands[i];
            band = std::make_unique<AnalyzerBand> (chainPosition, channel);
            addMouseListener (this, false);
            addChildComponent (band.get());

            auto& freqAttachment = freqAttachments[i];
            auto freqName = FilterInfo::getParameterName (i, channel, FilterInfo::FilterParam::FREQUENCY);
            freqAttachment = std::make_unique<ParametersAttachment> (*apvts.getParameter (freqName), nullptr);

            auto& qualityAttachment = qualityAttachments[i];
            auto qualityName = FilterInfo::getParameterName (i, channel, FilterInfo::FilterParam::Q);
            qualityAttachment = std::make_unique<ParametersAttachment> (*apvts.getParameter (qualityName), nullptr);

            auto& gainOrSlope = gainSlopeAttachments[i];
            auto gainOrSlopeName = shouldGetSlope (chainPosition)
                                       ? FilterInfo::getParameterName (i, channel, FilterInfo::FilterParam::SLOPE)
                                       : FilterInfo::getParameterName (i, channel, FilterInfo::FilterParam::GAIN);
            gainOrSlope = std::make_unique<ParametersAttachment> (*apvts.getParameter (gainOrSlopeName), nullptr);

            ++i;
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

void NodeController::refreshWidgets()
{
}

void NodeController::mouseDown (const juce::MouseEvent& e)
{
    /*
node: start drag, start gesture, notify listeners [DONE]
band: start drag, start gesture, notify listeners [DONE]
Q: start drag, start gesture, notify listeners [DONE]
controller: do nothing, notify listeners [DONE]
*/
}

void NodeController::mouseMove (const juce::MouseEvent& e)
{
    /*
       inactive Q controls:
node: highlight node, highlight band, notify listeners [DONE]
band: highlight node, highlight band, notify listeners [DONE]
controller: display band under cursor, notify listeners [DONE]
active Q controls:
node: highlight node, highlight band, notify listeners.  same a Q ? do nothing : hide Q controls [DONE]
band: highlight node, highlight band, notify listeners. same a Q ? do nothing : hide Q controls [DONE]
Q: highlight node, highlight band, highlight Q, notify listeners. [DONE]
controller: hide Q controls, highlight band/node inside mouseOverbounds, notify listeners [ DONE]
*/
}

void NodeController::mouseUp (const juce::MouseEvent& e)
{
    /*
       inactive Q controls:
node: highlight node, highlight band, end gesture, display Q controls, notify listeners [DONE]
band: highlight band, end gesture, notify listeners [DONE]
controller: do nothing, notify listeners [DONE]
active Q controls:
node: highlight node, highlight band, end gesture, notify listeners [DONE]
band: highlight band, end gesture, notify listeners [DONE]
Q: highlight Q, end gesture, notify listeners [DONE]
controller: notify listeners [DONE]
*/
}

void NodeController::mouseDrag (const juce::MouseEvent& e)
{
    /*
       inactive Q controls:
node: drag node, update parameter, notify listeners [DONE]
band: drag band, update parameter, notify listeners [DONE]
controller: do nothing, notify listeners [DONE]
active Q controls:
node: drag node, update parameter, notify listeners [DONE]
band: drag band, update parameter, notify listeners [DONE]
Q: drag Q, update parameter, notify listeners [DONE]
controller: do nothing, notify listeners [DONE]
*/
}

void NodeController::mouseEnter (const juce::MouseEvent& e)
{
    /*
       inactive Q controls:
node: highlight node, highlight band, notify listeners [DONE]
band: highlight node, highlight band, notify listeners [DONE]
controller: un-highlight everything, notify listeners [DONE]
active Q controls:
node: highlight node, highlight band, notify listeners [DONE]
band: highlight node, highlight band, notify listeners [DONE]
Q: change cursor, highlight node, highlight band, notify listeners [DONE]
controller: un-highlight everything, notify listeners [DONE]
*/
}

void NodeController::mouseExit (const juce::MouseEvent& e)
{
    /*
       inactive Q controls:
node: un-highlight node, notify listeners
band: un-highlight band, notify listeners
controller: un-highlight everything, notify listeners [DONE]
active Q controls:
node: un-highlight node, notify listeners
band: un-highlight band, notify listeners
Q: reset cursor, un-highlight Q, notify listeners
controller: un-highlight everything, notify listeners [DONE]
*/
}

void NodeController::mouseDoubleClick (const juce::MouseEvent& e)
{
    /*
       inactive Q controls:
node: reset freq/gain/slope
band: do nothing
controller: do nothing
active Q controls:
node: reset freq/gain
band: do nothing
Q: reset Q
controller: do nothing
*/
}

AnalyzerWidgetBase NodeController::getComponentForMouseEvent (const juce::MouseEvent& e)
{
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
