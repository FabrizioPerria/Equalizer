#include "data/ParameterAttachment.h"
#include "utils/MeterConstants.h"
#include <JuceHeader.h>
#include <functional>

ParametersAttachment::ParametersAttachment (juce::RangedAudioParameter& param,
                                            std::function<void (float)> parameterChangedCallback,
                                            juce::UndoManager* um)
    : parameter (param), undoManager (um), setValue (std::move (parameterChangedCallback))
{
    parameter.addListener (this);
    startTimerHz (FRAMES_PER_SECOND);
}

ParametersAttachment::~ParametersAttachment()
{
    stopTimer();
    parameter.removeListener (this);
}

void ParametersAttachment::sendInitialUpdate()
{
    parameterValueChanged ({}, parameter.getValue());
}

void ParametersAttachment::setValueAsCompleteGesture (float newDenormalisedValue)
{
    callIfParameterValueChanged (newDenormalisedValue,
                                 [this] (float f)
                                 {
                                     beginGesture();
                                     parameter.setValueNotifyingHost (f);
                                     endGesture();
                                 });
}

void ParametersAttachment::beginGesture()
{
    if (undoManager != nullptr)
        undoManager->beginNewTransaction();

    parameter.beginChangeGesture();
}

void ParametersAttachment::setValueAsPartOfGesture (float newDenormalisedValue)
{
    callIfParameterValueChanged (newDenormalisedValue, [this] (float f) { parameter.setValueNotifyingHost (f); });
}

void ParametersAttachment::endGesture()
{
    parameter.endChangeGesture();
}

void ParametersAttachment::resetToDefaultValue()
{
    //TODO: implement
}

juce::String ParametersAttachment::getName() const
{
    return "";
}

float ParametersAttachment::getDenormalizedValue() const
{
    return parameter.convertFrom0to1 (parameter.getValue());
}

juce::RangedAudioParameter& ParametersAttachment::getParameter()
{
    return parameter;
}

float ParametersAttachment::normalise (float f) const
{
    return parameter.convertTo0to1 (f);
}

void ParametersAttachment::parameterValueChanged (int, float newValue)
{
    lastValue = newValue;
    if (juce::MessageManager::getInstance()->isThisTheMessageThread() && setValue)
    {
        setValue (parameter.convertFrom0to1 (newValue));
    }
    else
    {
        lastValue.store (newValue);
        parameterChanged.set (true);
    }
}

void ParametersAttachment::parameterGestureChanged (int, bool)
{
}

void ParametersAttachment::timerCallback()
{
    if (parameterChanged.compareAndSetBool (false, true))
    {
        if (setValue)
        {
            setValue (parameter.convertFrom0to1 (lastValue));
        }
    }
}
