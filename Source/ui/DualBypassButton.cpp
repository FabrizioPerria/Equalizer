#include "ui/DualBypassButton.h"
#include "utils/EqParam.h"
#include "utils/PathFactory.h"

DualBypassButton::DualBypassButton (ChainPositions pos, AudioProcessorValueTreeState& apvtsToUse) : apvts (apvtsToUse), chainPosition (pos)
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
    auto bounds = getLocalBounds().toFloat();
    g.setColour (juce::Colours::aquamarine);
    g.drawRect (bounds);

    if (static_cast<EqMode> (apvts.getRawParameterValue ("eq_mode")->load()) != EqMode::STEREO)
    {
        auto center = bounds.getCentreX();
        auto top = bounds.getY();
        auto bottom = bounds.getBottom();

        juce::Line<float> line;
        line.setStart ({ center, top });
        line.setEnd ({ center, bottom });

        float dashLengths[2] = { 2.0f, 1.0f };
        g.drawDashedLine (line, dashLengths, 2.0f);
    }

    auto unflipped = juce::AffineTransform();
    auto flipped = unflipped.scale (-1.f, 1.f, bounds.getCentreX(), bounds.getCentreY());

    auto enableLeft = isShowingAsOn (Channel::LEFT);
    auto enableRight = isShowingAsOn (Channel::RIGHT);

    switch (chainPosition)
    {
        case ChainPositions::LOWCUT:
            PathDrawer::drawCutFilterSymbol (g, bounds, unflipped, enableLeft, enableRight);
            break;
        case ChainPositions::LOWSHELF:
            PathDrawer::drawShelfFilterSymbol (g, bounds, unflipped, enableLeft, enableRight);
            break;
        case ChainPositions::PEAK1:
        case ChainPositions::PEAK2:
        case ChainPositions::PEAK3:
        case ChainPositions::PEAK4:
            PathDrawer::drawPeakFilterSymbol (g, bounds, enableLeft, enableRight);
            break;
        case ChainPositions::HIGHSHELF:
            PathDrawer::drawShelfFilterSymbol (g, bounds, flipped, enableRight, enableLeft);
            break;
        case ChainPositions::HIGHCUT:
            PathDrawer::drawCutFilterSymbol (g, bounds, flipped, enableRight, enableLeft);
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
