#pragma once

#include "FilterParam.h"
#include <JuceHeader.h>

class MidSideProcessor
{
public:
    MidSideProcessor();
    ~MidSideProcessor();

    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();

        auto leftReader = inputBlock.getChannelPointer (Channel::LEFT);
        auto rightReader = inputBlock.getChannelPointer (Channel::RIGHT);

        auto leftWriter = outputBlock.getChannelPointer (Channel::LEFT);
        auto rightWriter = outputBlock.getChannelPointer (Channel::RIGHT);

        /* const auto correctiveGain = 1.0f / juce::MathConstants<float>::sqrt2; */
        const auto correctiveGain = juce::Decibels::decibelsToGain (-3.0f);

        for (int i = 0; i < outputBlock.getNumSamples(); ++i)
        {
            auto mid = (leftReader[i] + rightReader[i]) * correctiveGain;
            auto side = (leftReader[i] - rightReader[i]) * correctiveGain;

            leftWriter[i] = mid;
            rightWriter[i] = side;
        }
    }
};
