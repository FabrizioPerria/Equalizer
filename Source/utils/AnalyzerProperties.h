#pragma once

#include "utils/FFTDataGenerator.h"
#include <JuceHeader.h>
#include <map>

namespace AnalyzerProperties
{

enum class ParamNames
{
    EnableAnalyzer,
    AnalyzerDecayRate,
    AnalyzerPoints,
    AnalyzerProcessingMode
};

enum class ProcessingModes
{
    Pre,
    Post
};

inline const std::map<ParamNames, juce::String>& GetAnalyzerParams()
{
    static std::map<ParamNames, juce::String> map = { { ParamNames::EnableAnalyzer, "Enable Analyzer" },
                                                      { ParamNames::AnalyzerDecayRate, "Analyzer Decay Rate" },
                                                      { ParamNames::AnalyzerPoints, "Analyzer Points" },
                                                      { ParamNames::AnalyzerProcessingMode, "Analyzer Processing Mode" } };

    return map;
}

inline const std::map<FFTOrder, juce::String>& GetAnalyzerPoints()
{
    static std::map<FFTOrder, juce::String> map = { { FFTOrder::order2048, "2048" },
                                                    { FFTOrder::order4096, "4096" },
                                                    { FFTOrder::order8192, "8192" } };

    return map;
}

inline const std::map<ProcessingModes, juce::String>& GetProcessingModes()
{
    static std::map<ProcessingModes, juce::String> map = { { ProcessingModes::Pre, "Pre Eq" }, { ProcessingModes::Post, "Post Eq" } };

    return map;
}

inline void AddAnalyzerParams (juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    const auto& params = GetAnalyzerParams();

    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { params.at (ParamNames::EnableAnalyzer), 1 },
                                                            params.at (ParamNames::EnableAnalyzer),
                                                            true));

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { params.at (ParamNames::AnalyzerDecayRate), 2 },
                                                             params.at (ParamNames::AnalyzerDecayRate),
                                                             juce::NormalisableRange<float> { 0.0f, 30.0f, 1.0f, 1.0f },
                                                             30.0f));
    juce::StringArray orders;
    for (const auto& order : GetAnalyzerPoints())
    {
        orders.add (order.second);
    }
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { params.at (ParamNames::AnalyzerPoints), 3 },
                                                              params.at (ParamNames::AnalyzerPoints),
                                                              orders,
                                                              0));

    juce::StringArray modes;
    for (const auto& mode : GetProcessingModes())
    {
        modes.add (mode.second);
    }
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { params.at (ParamNames::AnalyzerProcessingMode), 4 },
                                                              params.at (ParamNames::AnalyzerProcessingMode),
                                                              modes,
                                                              0));
}

} // namespace AnalyzerProperties
