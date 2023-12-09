
#include "data/FilterParameters.h"

bool operator== (const FilterParametersBase& lhs, const FilterParametersBase& rhs)
{
    return juce::approximatelyEqual (lhs.frequency, rhs.frequency) && lhs.bypassed == rhs.bypassed
           && juce::approximatelyEqual (lhs.sampleRate, rhs.sampleRate);
}

bool operator!= (const FilterParametersBase& lhs, const FilterParametersBase& rhs)
{
    return ! (lhs == rhs);
}

bool operator== (const FilterParameters& lhs, const FilterParameters& rhs)
{
    return lhs.type == rhs.type //
           && juce::approximatelyEqual (lhs.gain, rhs.gain)
           && static_cast<const FilterParametersBase&> (lhs) == static_cast<const FilterParametersBase&> (rhs);
}

bool operator!= (const FilterParameters& lhs, const FilterParameters& rhs)
{
    return ! (lhs == rhs);
}

bool operator== (const HighCutLowCutParameters& lhs, const HighCutLowCutParameters& rhs)
{
    return lhs.order == rhs.order //
           && lhs.isLowCut == rhs.isLowCut
           && static_cast<const FilterParametersBase&> (lhs) == static_cast<const FilterParametersBase&> (rhs);
}

bool operator!= (const HighCutLowCutParameters& lhs, const HighCutLowCutParameters& rhs)
{
    return ! (rhs == lhs);
}
