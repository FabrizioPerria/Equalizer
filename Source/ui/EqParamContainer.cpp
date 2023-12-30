#include "ui/EqParamContainer.h"

EqParamContainer::EqParamContainer (juce::AudioProcessorValueTreeState& apvtsToUse)
    : apvts (apvtsToUse)
    , eqParamWidget0 (apvts, 0, true)
    , eqParamWidget1 (apvts, 1, false)
    , eqParamWidget2 (apvts, 2, false)
    , eqParamWidget3 (apvts, 3, false)
    , eqParamWidget4 (apvts, 4, false)
    , eqParamWidget5 (apvts, 5, false)
    , eqParamWidget6 (apvts, 6, false)
    , eqParamWidget7 (apvts, 7, true)
{
    addAndMakeVisible (eqParamWidget0);
    addAndMakeVisible (eqParamWidget1);
    addAndMakeVisible (eqParamWidget2);
    addAndMakeVisible (eqParamWidget3);
    addAndMakeVisible (eqParamWidget4);
    addAndMakeVisible (eqParamWidget5);
    addAndMakeVisible (eqParamWidget6);
    addAndMakeVisible (eqParamWidget7);
}

void EqParamContainer::resized()
{
    auto eqParamContainerBounds = getLocalBounds();
    auto eqParamWidgetWidth = eqParamContainerBounds.getWidth() / 8;
    eqParamWidget0.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
    eqParamWidget1.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
    eqParamWidget2.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
    eqParamWidget3.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
    eqParamWidget4.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
    eqParamWidget5.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
    eqParamWidget6.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
    eqParamWidget7.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
}

void EqParamContainer::paintOverChildren (juce::Graphics& g)
{
    g.setColour (juce::Colours::aquamarine);

    auto bounds = getLocalBounds().toFloat();
    bounds.removeFromBottom (static_cast<float> (buttonArea));
    g.drawRect (bounds);

    auto eqParamWidgetWidth = bounds.getWidth() / 8;

    // separate the 3 sliders for each band
    g.drawLine (bounds.getX(),
                bounds.getY() + EqParamWidget::sliderHeight,
                bounds.getRight(),
                bounds.getY() + EqParamWidget::sliderHeight,
                1);
    g.drawLine (bounds.getX(),
                bounds.getY() + 2 * EqParamWidget::sliderHeight,
                bounds.getRight(),
                bounds.getY() + 2 * EqParamWidget::sliderHeight,
                1);

    // separate the 8 bands vertically without drawing the first and the last line (they are already drawn by the drawRect above)
    while (bounds.getWidth() >= 2 * eqParamWidgetWidth)
    {
        auto eqParamWidgetBounds = bounds.removeFromLeft (eqParamWidgetWidth);
        g.drawLine (eqParamWidgetBounds.getRight(),
                    eqParamWidgetBounds.getY(),
                    eqParamWidgetBounds.getRight(),
                    eqParamWidgetBounds.getBottom(),
                    1);
    }
}
