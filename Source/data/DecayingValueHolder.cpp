#include "data/DecayingValueHolder.h"

DecayingValueHolder::DecayingValueHolder()
{
    setDecayRate (3);
    startTimerHz (FRAMES_PER_SECOND);
}

void DecayingValueHolder::updateHeldValue (float input)
{
    if (input > currentValue)
    {
        peakTime = getNow();
        currentValue = input;
        resetDecayRateMultiplier();
    }
}

float DecayingValueHolder::getCurrentValue() const
{
    return currentValue;
}

bool DecayingValueHolder::isOverThreshold() const
{
    return currentValue > threshold;
}

void DecayingValueHolder::setHoldTime (int ms)
{
    holdTime = ms;
}

void DecayingValueHolder::setDecayRate (float dbPerSec)
{
    decayRatePerFrame = dbPerSec / FRAMES_PER_SECOND;
}

void DecayingValueHolder::timerCallback()
{
    auto now = getNow();
    auto timeSincePeak = now - peakTime;
    if (timeSincePeak > holdTime)
    {
        auto decayRate = decayRatePerFrame * decayRateMultiplier;
        currentValue -= decayRate;
        currentValue = juce::jlimit (NEGATIVE_INFINITY, MAX_DECIBELS, currentValue);
        decayRateMultiplier *= decayAcceleration;
        if (currentValue <= NEGATIVE_INFINITY)
        {
            resetDecayRateMultiplier();
        }
    }
}

juce::int64 DecayingValueHolder::getNow()
{
    return juce::Time::currentTimeMillis();
}

void DecayingValueHolder::resetDecayRateMultiplier()
{
    decayRateMultiplier = 1;
}
