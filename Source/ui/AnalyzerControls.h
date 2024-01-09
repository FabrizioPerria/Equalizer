#pragma once

#include <JuceHeader.h>

struct AnalyzerControlsLookAndFeel : LookAndFeel_V4
{
    void drawRotarySlider (Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           const float rotaryStartAngle,
                           const float rotaryEndAngle,
                           Slider& slider) override;

    void drawLinearSlider (Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           const Slider::SliderStyle style,
                           Slider& slider) override;

private:
    juce::Colour darkdark = Colour::fromString (JUCE_LIVE_CONSTANT ("ff0d0e16"));
    juce::Colour dark = Colour::fromString (JUCE_LIVE_CONSTANT ("ff14141f"));
    juce::Colour light = Colour::fromString (JUCE_LIVE_CONSTANT ("88367e3b3"));
};

struct VerticalSwitch : juce::Component
{
    VerticalSwitch (juce::RangedAudioParameter& rap);
    ~VerticalSwitch() override;
    juce::StringArray options;

private:
    AnalyzerControlsLookAndFeel laf;
    juce::RangedAudioParameter* rap;
};

struct KnobWithLabels : juce::Slider
{
    KnobWithLabels() : Slider (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        setLookAndFeel (&lnf);
    }
    ~KnobWithLabels() override
    {
        setLookAndFeel (nullptr);
    }

    void paint (juce::Graphics& g)
    {
        auto startAng = degreesToRadians (180.f + 45.f);
        auto endAng = degreesToRadians (180.f - 45.f) + MathConstants<float>::twoPi;

        auto range = getRange();

        auto sliderBounds = getSliderBounds();

        getLookAndFeel().drawRotarySlider (g,
                                           sliderBounds.getX(),
                                           sliderBounds.getY(),
                                           sliderBounds.getWidth(),
                                           sliderBounds.getHeight(),
                                           jmap (getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                           startAng,
                                           endAng,
                                           *this);

        auto center = sliderBounds.toFloat().getCentre();
        auto radius = sliderBounds.getWidth() * 0.5f;

        g.setColour (Colour (0u, 172u, 1u));
        g.setFont (getTextHeight());

        auto numChoices = labels.size();
        for (int i = 0; i < numChoices; ++i)
        {
            auto pos = i * (1.f / (numChoices - 1));
            jassert (0.f <= pos);
            jassert (pos <= 1.f);

            auto ang = jmap (pos, 0.f, 1.f, startAng, endAng);

            auto c = center.getPointOnCircumference (radius + JUCE_LIVE_CONSTANT (7), ang);

            Rectangle<float> r;
            auto str = labels[i];
            r.setSize (g.getCurrentFont().getStringWidth (str), getTextHeight());
            r.setCentre (c);
            r.setY (r.getY() + JUCE_LIVE_CONSTANT (4));

            g.drawFittedText (str, r.toNearestInt(), juce::Justification::centred, 1);
        }
    }

    juce::Array<juce::String> labels;

    juce::Rectangle<int> getSliderBounds() const
    {
        auto bounds = getLocalBounds();

        auto size = juce::jmin (bounds.getWidth(), bounds.getHeight());

        size -= getTextHeight() * 2;
        juce::Rectangle<int> r;
        r.setSize (size, size);
        r.setCentre (bounds.getCentreX(), 0);
        r.setY (2);

        return r;
    }

    int getTextHeight() const
    {
        return 14;
    }

private:
    AnalyzerControlsLookAndFeel lnf;

    juce::RangedAudioParameter* param;
};

struct AnalyzerControls : juce::Component
{
    AnalyzerControls (juce::AudioProcessorValueTreeState& apv);
    ~AnalyzerControls() override;

    void resized() override;

private:
    void toggleEnable();

    AnalyzerControlsLookAndFeel laf;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    juce::TextButton enableButton { "On" };
    std::unique_ptr<ButtonAttachment> enableButtonAttachment;
    juce::Slider inputSlider;
    std::unique_ptr<SliderAttachment> inputSliderAttachment;
    juce::Slider pointsSlider;
    std::unique_ptr<SliderAttachment> pointsSliderAttachment;
    KnobWithLabels decaySlider;
    std::unique_ptr<SliderAttachment> decaySliderAttachment;
};
