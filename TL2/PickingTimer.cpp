#include "pch.h"
#include "PickingTimer.h"

// Static member initialization
double FWindowsPlatformTime::GSecondsPerCycle = 0.0;
bool FWindowsPlatformTime::bInitialized = false;