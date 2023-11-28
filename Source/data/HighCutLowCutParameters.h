#pragma once

#include "data/FilterParametersBase.h"

struct HighCutLowCutParameters : public FilterParametersBase
{
	int order = 1;
	bool isLowCut = false;
};
