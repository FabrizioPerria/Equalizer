#include "PluginProcessor.h"
#include "ui/BypassButton.h"
#include <JuceHeader.h>

struct GlobalBypassButton : BypassButton, juce::Timer
{
    GlobalBypassButton (EqualizerAudioProcessor& p);
    void timerCallback() override;
    void clicked() override;
    void paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
    EqualizerAudioProcessor& audioProcessor;
};
