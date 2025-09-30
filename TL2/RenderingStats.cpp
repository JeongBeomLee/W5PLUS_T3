#include "pch.h"
#include "RenderingStats.h"
#include <algorithm>

URenderingStatsCollector& URenderingStatsCollector::GetInstance()
{
    static URenderingStatsCollector Instance;
    return Instance;
}

void URenderingStatsCollector::UpdateFrameStats(const FRenderingStats& InNewStats)
{
    if (!bEnabled)
    {
        return;
    }
        
    CurrentFrameStats = InNewStats;
    
    // 프레임 히스토리에 추가
    FrameHistory[FrameHistoryIndex] = InNewStats;
    FrameHistoryIndex = (FrameHistoryIndex + 1) % AVERAGE_FRAME_COUNT;
    ValidFrameCount = std::min(ValidFrameCount + 1, AVERAGE_FRAME_COUNT);
    
    // 평균 계산
    CalculateAverageStats();
}

void URenderingStatsCollector::BeginFrame()
{
    if (!bEnabled)
    {
        return;
    }
        
    FrameStartTime = std::chrono::high_resolution_clock::now();
    CurrentFrameStats.Reset();
}

void URenderingStatsCollector::EndFrame()
{
    if (!bEnabled)
    {
        return;
    }
        
    auto EndTime = std::chrono::high_resolution_clock::now();
    auto Duration = std::chrono::duration_cast<std::chrono::microseconds>(EndTime - FrameStartTime);
    CurrentFrameStats.TotalRenderTime = Duration.count() / 1000.0f; // ms로 변환
}

void URenderingStatsCollector::UpdatePickingStats(double InPickingTimeMs)
{
    LastPickingTimeMs = InPickingTimeMs;
    NumPickingAttempts++;
    AccumulatedPickingTimeMs += InPickingTimeMs;
}

double URenderingStatsCollector::GetLastPickingTime() const
{
    return LastPickingTimeMs;
}

uint64_t URenderingStatsCollector::GetNumPickingAttempts() const
{
    return NumPickingAttempts;
}

double URenderingStatsCollector::GetAccumulatedPickingTime() const
{
    return AccumulatedPickingTimeMs;
}

void URenderingStatsCollector::ResetStats()
{
    CurrentFrameStats.Reset();
    AverageStats.Reset();
    
    for (uint32 i = 0; i < AVERAGE_FRAME_COUNT; ++i)
    {
        FrameHistory[i].Reset();
    }
    
    FrameHistoryIndex = 0;
    ValidFrameCount = 0;
}

void URenderingStatsCollector::CalculateAverageStats()
{
    if (ValidFrameCount == 0)
        return;
        
    AverageStats.Reset();
    
    // 모든 유효한 프레임의 평균 계산
    for (uint32 i = 0; i < ValidFrameCount; ++i)
    {
        const FRenderingStats& Frame = FrameHistory[i];
        
        AverageStats.TotalDrawCalls += Frame.TotalDrawCalls;
        AverageStats.MaterialChanges += Frame.MaterialChanges;
        AverageStats.TextureChanges += Frame.TextureChanges;
        AverageStats.ShaderChanges += Frame.ShaderChanges;
        AverageStats.BasePassDrawCalls += Frame.BasePassDrawCalls;
        AverageStats.DepthPrePassDrawCalls += Frame.DepthPrePassDrawCalls;
        AverageStats.TranslucentPassDrawCalls += Frame.TranslucentPassDrawCalls;
        AverageStats.DebugPassDrawCalls += Frame.DebugPassDrawCalls;
        AverageStats.TotalRenderTime += Frame.TotalRenderTime;
    }
    
    // 평균 계산
    float InvCount = 1.0f / static_cast<float>(ValidFrameCount);
    AverageStats.TotalDrawCalls = static_cast<uint32>(AverageStats.TotalDrawCalls * InvCount);
    AverageStats.MaterialChanges = static_cast<uint32>(AverageStats.MaterialChanges * InvCount);
    AverageStats.TextureChanges = static_cast<uint32>(AverageStats.TextureChanges * InvCount);
    AverageStats.ShaderChanges = static_cast<uint32>(AverageStats.ShaderChanges * InvCount);
    AverageStats.BasePassDrawCalls = static_cast<uint32>(AverageStats.BasePassDrawCalls * InvCount);
    AverageStats.DepthPrePassDrawCalls = static_cast<uint32>(AverageStats.DepthPrePassDrawCalls * InvCount);
    AverageStats.TranslucentPassDrawCalls = static_cast<uint32>(AverageStats.TranslucentPassDrawCalls * InvCount);
    AverageStats.DebugPassDrawCalls = static_cast<uint32>(AverageStats.DebugPassDrawCalls * InvCount);
    AverageStats.TotalRenderTime *= InvCount;
}