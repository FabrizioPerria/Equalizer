#include "ui/EqParamContainer.h"

EqParamContainer::EqParamContainer (juce::AudioProcessorValueTreeState& apvtsToUse) : apvts (apvtsToUse)
{
    addAndMakeVisible (lowcutWidget);
    addAndMakeVisible (lowShelfWidget);
    addAndMakeVisible (peakWidget1);
    addAndMakeVisible (peakWidget2);
    addAndMakeVisible (peakWidget3);
    addAndMakeVisible (peakWidget4);
    addAndMakeVisible (highShelfWidget);
    addAndMakeVisible (highCutWidget);
}

void EqParamContainer::resized()
{
    auto eqParamContainerBounds = getLocalBounds();
    auto eqParamWidgetWidth = eqParamContainerBounds.getWidth() / 8;
    lowcutWidget.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
    lowShelfWidget.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
    peakWidget1.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
    peakWidget2.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
    peakWidget3.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
    peakWidget4.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
    highShelfWidget.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
    highCutWidget.setBounds (eqParamContainerBounds.removeFromLeft (eqParamWidgetWidth));
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
