#pragma once

#include "utils/Fifo.h"
#include <JuceHeader.h>

template <typename CoefficientType>
struct ReleasePool : juce::Timer
{
    using Ptr = juce::ReferenceCountedObjectPtr<CoefficientType>;

    ReleasePool()
    {
        deletionPool.reserve (FIFO_SIZE);
        startTimer (TIMER_INTERVAL_MS);
    }

    void add (Ptr ptr)
    {
        if (juce::MessageManager::getInstanceWithoutCreating()->isThisTheMessageThread())
        {
            addIfNotAlreadyThere (ptr);
        }
        else
        {
            if (deletionFifo.push (ptr))
            {
                itemsToDelete = true;
            }
            else
            {
                jassertfalse; // If this fails, it means that the FIFO is too small
            }
        }
    }

    void timerCallback() override
    {
        if (itemsToDelete.compareAndSetBool (false, true))
        {
            Ptr dataToDelete;
            while (deletionFifo.getNumAvailableForReading() > 0)
            {
                auto exchanged = deletionFifo.exchange (std::move (dataToDelete));
                jassert (exchanged);
                if (dataToDelete.get() != nullptr)
                {
                    addIfNotAlreadyThere (dataToDelete);
                    dataToDelete = nullptr;
                }
            }
        }

        deletionPool.erase (std::remove_if (deletionPool.begin(), //
                                            deletionPool.end(),
                                            [] (const Ptr& ptr) { return ptr.get()->getReferenceCount() <= 1; }),
                            deletionPool.end());
    }

private:
    void addIfNotAlreadyThere (Ptr ptr)
    {
        if (std::find (deletionPool.begin(), deletionPool.end(), ptr) == deletionPool.end())
        {
            deletionPool.push_back (ptr);
        }
    }

    static const int TIMER_INTERVAL_MS = 2000;
    static const int FIFO_SIZE = 50000;

    std::vector<Ptr> deletionPool;

    Fifo<Ptr, FIFO_SIZE> deletionFifo;

    juce::Atomic<bool> itemsToDelete { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReleasePool)
};
