#include "ui/BypassButtonContainer.h"
#include "PluginProcessor.h"
#include "utils/PathFactory.h"

BypassButtonContainer::BypassButtonContainer (juce::AudioProcessorValueTreeState& apvts)
{
    auto index = 0;
    for (auto& button : bypassButtons)
    {
        button = std::make_unique<DualBypassButton> (static_cast<ChainPositions> (index), apvts);
        addAndMakeVisible (*button);
        index++;
    }
}

void BypassButtonContainer::paint (juce::Graphics& g)
{
}

void BypassButtonContainer::resized()
{
    auto height = getLocalBounds().getHeight();

    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::row;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;

    for (auto& button : bypassButtons)
    {
        flexBox.items.add (juce::FlexItem (*button).withMinWidth (height * 3).withMinHeight (height));
    }

    flexBox.performLayout (getLocalBounds().toFloat());
}
