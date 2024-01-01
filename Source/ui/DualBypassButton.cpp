#include "ui/DualBypassButton.h"
#include "utils/EqParam.h"

DualBypassButton::DualBypassButton (ChainPositions pos, AudioProcessorValueTreeState& apvtsToUse) : chainPosition (pos), apvts (apvtsToUse)
{
    addAndMakeVisible (leftButton);
    addAndMakeVisible (rightButton);

    auto leftParamName = FilterInfo::getParameterName (static_cast<int> (pos), Channel::LEFT, FilterInfo::FilterParam::BYPASS);
    auto rightParamName = FilterInfo::getParameterName (static_cast<int> (pos), Channel::RIGHT, FilterInfo::FilterParam::BYPASS);

    leftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, leftParamName, leftButton);
    rightAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, rightParamName, rightButton);

    leftButton.onLeft = true;
    rightButton.onLeft = false;
    rightButton.isPaired = true;

    leftButton.onClick = [this] { this->repaint(); };
    rightButton.onClick = [this] { this->repaint(); };

    auto modeParam = apvts.getParameter ("eq_mode");
    auto safePtr = juce::Component::SafePointer<DualBypassButton> (this);
    eqModeListener = std::make_unique<ParamListener<float>> (modeParam,
                                                             [safePtr] (float v)
                                                             {
                                                                 if (auto* comp = safePtr.getComponent())
                                                                 {
                                                                     comp->refreshButtons (static_cast<EqMode> (v));
                                                                 }
                                                             });
}

void DualBypassButton::paintOverChildren (juce::Graphics& g)
{
    //TODO: handle colors
    //TODO: draw dotted line if not stereo
    //TODO: get path from factory
    switch (chainPosition)
    {
        case ChainPositions::LOWCUT:
            break;
        case ChainPositions::LOWSHELF:
            break;
        case ChainPositions::PEAK1:
            break;
        case ChainPositions::PEAK2:
            break;
        case ChainPositions::PEAK3:
            break;
        case ChainPositions::PEAK4:
            break;
        case ChainPositions::HIGHSHELF:
            break;
        case ChainPositions::HIGHCUT:
            break;
        default:
            jassertfalse;
    }
}

void DualBypassButton::resized()
{
    auto mode = static_cast<EqMode> (apvts.getRawParameterValue ("eq_mode")->load());
    refreshButtons (mode);
}

void DualBypassButton::refreshButtons (EqMode mode)
{
    auto bounds = getLocalBounds();
    if (mode != EqMode::STEREO)
    {
        rightButton.setVisible (true);

        leftButton.isPaired = true;
        leftButton.setBounds (bounds.removeFromLeft (getWidth() / 2));

        rightButton.setBounds (bounds);
    }
    else
    {
        leftButton.isPaired = false;
        leftButton.setBounds (bounds);

        rightButton.setVisible (false);
    }
}

void DualBypassButton::initializeParamListeners()
{
}

bool DualBypassButton::isShowingAsOn (Channel channel)
{
    if (channel == Channel::LEFT)
    {
        return leftButton.isShowingAsOn();
    }

    return rightButton.isShowingAsOn();
}
