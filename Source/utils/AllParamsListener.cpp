#include "utils/AllParamsListener.h"
#include "utils/MeterConstants.h"

AllParamsListener::AllParamsListener (juce::AudioProcessorValueTreeState* apv, std::function<void()> f) : apvts (apv), func (f)
{
    auto params = apvts->processor.getParameters();
    for (auto param : params)
        param->addListener (this);

    startTimerHz (FRAMES_PER_SECOND);
}

AllParamsListener::~AllParamsListener()
{
    stopTimer();
    auto params = apvts->processor.getParameters();
    for (auto param : params)
        param->removeListener (this);
}

void AllParamsListener::timerCallback()
{
    if (changed.compareAndSetBool (false, true))
        func();
}

void AllParamsListener::parameterValueChanged (int /*parameterIndex*/, float /*newValue*/)
{
    changed = true;
}

void AllParamsListener::parameterGestureChanged (int /*parameterIndex*/, bool /*gestureIsStarting*/)
{
}
