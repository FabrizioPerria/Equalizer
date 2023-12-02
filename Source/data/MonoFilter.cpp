#include "data/MonoFilter.h"
#include "data/FilterParameters.h"
#include "utils/CoefficientsMaker.h"

static FilterParameters oldFilterParams[NUM_FILTERS];
static HighCutLowCutParameters oldHighCutLowCutParams[NUM_FILTERS];

static void setBypassed (MonoFilter& filter, int filterIndex, bool bypassed)
{
    jassert (filterIndex < 0 || filterIndex >= NUM_FILTERS);
    switch (filterIndex)
    {
        case 0:
            filter.setBypassed<0> (bypassed);
            break;

        default:
            jassertfalse;
            break;
    }
}

static void updateCoefficients (MonoFilter& filter, int filterIndex, Coefficients coefficients)
{
    jassert (filterIndex < 0 || filterIndex >= NUM_FILTERS);
    switch (filterIndex)
    {
        case 0:
            *filter.get<0>().coefficients = *coefficients;
            break;

        default:
            jassertfalse;
            break;
    }
}

template <typename ParamsType, typename ParamsGetter>
static void updateFilter (juce::AudioProcessorValueTreeState& apvts,
                          int i,
                          double sampleRate,
                          FilterInfo::FilterType filterType,
                          ParamsType& oldParams,
                          ParamsGetter getParameters,
                          MonoFilter& leftChannel,
                          MonoFilter& rightChannel)
{
    auto params = getParameters (apvts, i, filterType, sampleRate);
    if (params != oldParams[i])
    {
        setBypassed (leftChannel, i, params.bypassed);
        setBypassed (rightChannel, i, params.bypassed);
        auto coefficients = CoefficientsMaker<float>::make (params, sampleRate);
        updateCoefficients (leftChannel, i, coefficients);
        updateCoefficients (rightChannel, i, coefficients);
        oldParams[i] = params;
    }
}

void updateFilters (juce::AudioProcessorValueTreeState& apvts,
                    MonoFilter& leftChannel,
                    MonoFilter& rightChannel,
                    double sampleRate)
{
    for (int i = 0; i < NUM_FILTERS; ++i)
    {
        auto filterType = FilterInfo::getFilterType (apvts, i);

        if (needsParametricParams (filterType))
        {
            updateFilter (apvts,
                          i,
                          sampleRate,
                          filterType,
                          oldFilterParams,
                          FilterParameters::getParameters,
                          leftChannel,
                          rightChannel);
        }
        else
        {
            updateFilter (apvts,
                          i,
                          sampleRate,
                          filterType,
                          oldHighCutLowCutParams,
                          HighCutLowCutParameters::getParameters,
                          leftChannel,
                          rightChannel);
        }
    }
}