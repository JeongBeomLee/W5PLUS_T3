#pragma once
#include <Windows.h>

class FWindowsPlatformTime
{
public:
    static double GSecondsPerCycle; // 0
    static bool bInitialized; // false

    static void InitTiming()
    {
        if (!bInitialized)
        {
            bInitialized = true;

            double Frequency = (double)GetFrequency();
            if (Frequency <= 0.0)
            {
                Frequency = 1.0;
            }

            GSecondsPerCycle = 1.0 / Frequency;
        }
    }
    static float GetSecondsPerCycle()
    {
        if (!bInitialized)
        {
            InitTiming();
        }
        return (float)GSecondsPerCycle;
    }
    static uint64_t GetFrequency()
    {
        LARGE_INTEGER Frequency;
        QueryPerformanceFrequency(&Frequency);
        return Frequency.QuadPart;
    }
    static double ToMilliseconds(uint64_t CycleDiff)
    {
        double Ms = static_cast<double>(CycleDiff)
            * GetSecondsPerCycle()
            * 1000.0;

        return Ms;
    }

    static uint64_t Cycles64()
    {
        LARGE_INTEGER CycleCount;
        QueryPerformanceCounter(&CycleCount);
        return (uint64_t)CycleCount.QuadPart;
    }
};

struct TStatId
{
};

typedef FWindowsPlatformTime FPlatformTime;

class FScopeCycleCounter
{
public:
    FScopeCycleCounter(TStatId StatId)
        : StartCycles(FPlatformTime::Cycles64())
        , UsedStatId(StatId)
    {
    }

    ~FScopeCycleCounter()
    {
        Finish();
    }

    uint64_t Finish()
    {
        const uint64_t EndCycles = FPlatformTime::Cycles64();
        const uint64_t CycleDiff = EndCycles - StartCycles;

        // FThreadStats::AddMessage(UsedStatId, EStatOperation::Add, CycleDiff);

        return CycleDiff;
    }

private:
    uint64_t StartCycles;
    TStatId UsedStatId;
};