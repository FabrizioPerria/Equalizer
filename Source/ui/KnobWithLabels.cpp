#include "ui/KnobWithLabels.h"

KnobWithLabels::KnobWithLabels (juce::String newTitle)
    : Slider (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox), title (newTitle)
{
    setLookAndFeel (&lnf);
    onValueChange = [this] { updateTooltip(); };
}

KnobWithLabels::~KnobWithLabels()
{
    setLookAndFeel (nullptr);
}

void KnobWithLabels::paint (juce::Graphics& g)
{
    auto startAng = juce::degreesToRadians (180.f + 45.f);
    auto endAng = juce::degreesToRadians (180.f - 45.f) + juce::MathConstants<float>::twoPi;

    auto range = getRange();

    auto bounds = getLocalBounds();
    auto titleArea = bounds.removeFromTop (textHeight);
    auto sliderBounds = bounds;
    g.setColour (juce::Colours::aquamarine);
    g.setFont (textHeight);
    g.drawFittedText (title, titleArea, juce::Justification::centred, 1);
    sliderBounds.removeFromTop (textHeight);
    sliderBounds.removeFromBottom (textHeight);

    auto value = static_cast<float> (getValue());
    auto rangeStart = static_cast<float> (range.getStart());
    auto rangeEnd = static_cast<float> (range.getEnd());
    getLookAndFeel().drawRotarySlider (g,
                                       sliderBounds.getX(),
                                       sliderBounds.getY(),
                                       sliderBounds.getWidth(),
                                       sliderBounds.getHeight(),
                                       juce::jmap (value, rangeStart, rangeEnd, 0.0f, 1.0f),
                                       startAng,
                                       endAng,
                                       *this);

    auto center = sliderBounds.toFloat().getCentre();

    g.setColour (juce::Colour (0u, 172u, 1u));
    g.setFont (10);

    auto numChoices = labels.size();
    for (int i = 0; i < numChoices; ++i)
    {
        auto pos = i * (1.f / (numChoices - 1));
        jassert (0.f <= pos);
        jassert (pos <= 1.f);

        auto ang = juce::jmap (pos, 0.f, 1.f, startAng, endAng);

        auto c = center.getPointOnCircumference (35, ang);

        juce::Rectangle<float> r;
        auto str = labels[i];
        r.setSize (g.getCurrentFont().getStringWidth (str), 10);
        r.setCentre (c);
        r.setY (r.getY());

        g.drawFittedText (str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

void KnobWithLabels::updateTooltip()
{
    setTooltip (std::to_string (getValue()));
}
