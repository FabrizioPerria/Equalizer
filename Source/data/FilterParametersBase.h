#pragma once

struct FilterParametersBase
{
    float frequency = 440.0f;
    bool bypassed = false;
    float quality = 1.0f;
    double sampleRate = 44100.0;
};
