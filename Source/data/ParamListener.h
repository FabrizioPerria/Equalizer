#pragma once

#include "utils/MeterConstants.h"
#include <JuceHeader.h>

template <typename ValueType>
struct ParamListener : juce::Timer
{
    ParamListener (juce::RangedAudioParameter* paramToUse, std::function<void (ValueType)> callbackToUse)
    {
        if (paramToUse != nullptr)
        {
            param = paramToUse;
        }
        else
        {
            jassertfalse;
        }

        if (callbackToUse != nullptr)
        {
            callback = callbackToUse;
        }
        else
        {
            jassertfalse;
        }

        value = param->getValue();
        startTimerHz (FRAMES_PER_SECOND);
    }

    void timerCallback() override
    {
        auto newValue = param->getValue();

        if (newValue != value)
        {
            value = newValue;
            callback (param->convertFrom0to1 (value));
        }
    }

private:
    ValueType value;
    juce::RangedAudioParameter* param;

    std::function<void (ValueType)> callback;
};
