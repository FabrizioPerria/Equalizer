#pragma once

#include "utils/Decibel.h"

struct MeterValues
{
    Decibel<float> leftPeakDb, rightPeakDb, leftRmsDb, rightRmsDb;
};
