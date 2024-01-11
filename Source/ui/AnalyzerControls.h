#pragma once

#include <JuceHeader.h>

struct AnalyzerControlsLookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           const float rotaryStartAngle,
                           const float rotaryEndAngle,
                           juce::Slider& slider) override;

    void drawLinearSlider (juce::Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           const juce::Slider::SliderStyle style,
                           juce::Slider& slider) override;

private:
    /* juce::Colour darkdark = juce::Colour::fromString (JUCE_LIVE_CONSTANT ("ff0d0e16")); */
    /* juce::Colour dark = juce::Colour::fromString (JUCE_LIVE_CONSTANT ("ff14141f")); */
    /* juce::Colour light = juce::Colour::fromString (JUCE_LIVE_CONSTANT ("88367e3b3")); */
    juce::Colour darkdark = juce::Colour::fromString (("ff0d0e16"));
    juce::Colour dark = juce::Colour::fromString (("ff14141f"));
    juce::Colour light = juce::Colour::fromString (("88367e3b3"));
};

struct VerticalSwitch : juce::Slider
{
    VerticalSwitch (juce::String newTitle)
        : Slider (juce::Slider::SliderStyle::LinearVertical, juce::Slider::TextEntryBoxPosition::NoTextBox), title (newTitle)
    {
        setLookAndFeel (&laf);
    }

    ~VerticalSwitch() override
    {
        setLookAndFeel (nullptr);
    }

    void paint (juce::Graphics& g) override
    {
        auto range = getRange();
        auto bounds = getLocalBounds().toDouble();
        bounds.removeFromLeft (5);
        auto titleArea = bounds.removeFromTop (12);

        g.setColour (juce::Colours::aquamarine);
        g.setFont (12);
        g.drawFittedText (title, titleArea.toNearestInt(), juce::Justification::left, 1);

        bounds.removeFromTop (12);
        bounds.removeFromBottom (12);
        auto normalized = juce::jmap (getValue(), range.getStart(), range.getEnd(), bounds.getBottom(), bounds.getY());

        getLookAndFeel().drawLinearSlider (g,
                                           bounds.getX(),
                                           bounds.getY(),
                                           bounds.getWidth(),
                                           bounds.getHeight(),
                                           normalized,
                                           0,
                                           1,
                                           getSliderStyle(),
                                           *this);

        auto labelArea = bounds;

        labelArea.removeFromLeft (JUCE_LIVE_CONSTANT (12));
        labelArea.setHeight (10);
        g.setColour (juce::Colours::aquamarine);
        g.setFont (10);
        auto numChoices = labels.size();

        for (int i = 0; i < numChoices; ++i)
        {
            auto ypos = bounds.getBottom() - 5 - i * (bounds.getHeight() / (numChoices - 1));
            g.setColour (juce::Colours::green);
            g.drawFittedText (labels[i], labelArea.withY (ypos).toNearestInt(), juce::Justification::left, 1);
        }
    }

    juce::StringArray labels;

private:
    AnalyzerControlsLookAndFeel laf;

    juce::String title;

    juce::Rectangle<int> getSliderBounds() const
    {
        juce::Rectangle<int> r = getLocalBounds();

        return r;
    }
};

struct KnobWithLabels : juce::Slider
{
    KnobWithLabels (juce::String newTitle)
        : Slider (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox), title (newTitle)
    {
        setLookAndFeel (&lnf);
    }
    ~KnobWithLabels() override
    {
        setLookAndFeel (nullptr);
    }

    void paint (juce::Graphics& g) override
    {
        auto startAng = juce::degreesToRadians (180.f + 45.f);
        auto endAng = juce::degreesToRadians (180.f - 45.f) + juce::MathConstants<float>::twoPi;

        auto range = getRange();

        auto bounds = getLocalBounds();
        auto titleArea = bounds.removeFromTop (getTextHeight());
        auto sliderBounds = bounds;
        g.setColour (juce::Colours::aquamarine);
        g.setFont (getTextHeight());
        g.drawFittedText (title, titleArea, juce::Justification::centred, 1);
        sliderBounds.removeFromTop (12);
        sliderBounds.removeFromBottom (12);

        getLookAndFeel().drawRotarySlider (g,
                                           sliderBounds.getX(),
                                           sliderBounds.getY(),
                                           sliderBounds.getWidth(),
                                           sliderBounds.getHeight(),
                                           juce::jmap (getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                           startAng,
                                           endAng,
                                           *this);

        auto center = sliderBounds.toFloat().getCentre();

        g.setColour (juce::Colour (0u, 172u, 1u));
        g.setFont (getTextHeight());

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
            r.setSize (g.getCurrentFont().getStringWidth (str), getTextHeight());
            r.setCentre (c);
            r.setY (r.getY());

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
        return 12;
    }

private:
    AnalyzerControlsLookAndFeel lnf;
    juce::String title;
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
    VerticalSwitch inputSlider { "Input" };
    std::unique_ptr<SliderAttachment> inputSliderAttachment;
    VerticalSwitch pointsSlider { "Points" };
    std::unique_ptr<SliderAttachment> pointsSliderAttachment;
    KnobWithLabels decaySlider { "Decay Rate" };
    std::unique_ptr<SliderAttachment> decaySliderAttachment;
};
