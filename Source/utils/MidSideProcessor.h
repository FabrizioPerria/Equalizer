#pragma once

#include "utils/EqParam.h"
#include <JuceHeader.h>

struct MidSideProcessor
{
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();

        auto leftReader = inputBlock.getChannelPointer (static_cast<size_t> (Channel::LEFT));
        auto rightReader = inputBlock.getChannelPointer (static_cast<size_t> (Channel::RIGHT));

        auto leftWriter = outputBlock.getChannelPointer (static_cast<size_t> (Channel::LEFT));
        auto rightWriter = outputBlock.getChannelPointer (static_cast<size_t> (Channel::RIGHT));

        /* const auto correctiveGain = 1.0f / juce::MathConstants<float>::sqrt2; */
        const auto correctiveGain = juce::Decibels::decibelsToGain (-3.0f);

        for (size_t i = 0; i < outputBlock.getNumSamples(); ++i)
        {
            auto mid = (leftReader[i] + rightReader[i]) * correctiveGain;
            auto side = (leftReader[i] - rightReader[i]) * correctiveGain;

            leftWriter[i] = mid;
            rightWriter[i] = side;
        }
    }
};
