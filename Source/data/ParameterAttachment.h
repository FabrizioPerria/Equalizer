#pragma once

#include <JuceHeader.h>
#include <functional>

struct ParametersAttachment : juce::AudioProcessorParameter::Listener, juce::Timer
{
    ParametersAttachment (juce::RangedAudioParameter& param,
                          std::function<void (float)> parameterChangedCallback,
                          juce::UndoManager* um = nullptr);
    ~ParametersAttachment() override;
    void sendInitialUpdate();
    void setValueAsCompleteGesture (float newDenormalisedValue);
    void beginGesture();
    void setValueAsPartOfGesture (float newDenormalisedValue);
    void endGesture();
    void resetToDefaultValue();
    juce::String getName() const;
    float getDenormalizedValue() const;
    juce::RangedAudioParameter& getParameter();

private:
    float normalise (float f) const;

    template <typename Callback>
    void callIfParameterValueChanged (float newDenormalisedValue, Callback&& callback)
    {
        const auto newValue = normalise (newDenormalisedValue);

        if (parameter.getValue() != newValue)
            callback (newValue);
    }

    void parameterValueChanged (int, float newValue) override;
    void parameterGestureChanged (int, bool) override;
    void timerCallback() override;

    juce::RangedAudioParameter& parameter;
    std::atomic<float> lastValue { 0.0f };
    juce::UndoManager* undoManager = nullptr;
    std::function<void (float)> setValue;
    juce::Atomic<bool> parameterChanged { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametersAttachment)
};
