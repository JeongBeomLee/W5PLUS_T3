#pragma once

/**
 * @brief 렌더링 통계 정보 구조체
 */
struct FRenderingStats
{
    // DrawCall 통계
    uint32 TotalDrawCalls = 0;
    uint32 MaterialChanges = 0;
    uint32 TextureChanges = 0;
    uint32 ShaderChanges = 0;

    // RenderPass 통계
    uint32 BasePassDrawCalls = 0;
    uint32 DepthPrePassDrawCalls = 0;
    uint32 TranslucentPassDrawCalls = 0;
    uint32 DebugPassDrawCalls = 0;

    // 성능 통계
    float TotalRenderTime = 0.0f;      // ms (전체 프레임 시간)
    // float BasePassTime = 0.0f;         // ms (미사용 - RenderPass 시스템 미구현)
    // float SortTime = 0.0f;             // ms (미사용 - DrawCall 정렬 미구현)

    // 피킹 시간 통계
    float PickingTime = 0.0f;


    void Reset()
    {
        TotalDrawCalls = 0;
        MaterialChanges = 0;
        TextureChanges = 0;
        ShaderChanges = 0;
        BasePassDrawCalls = 0;
        DepthPrePassDrawCalls = 0;
        TranslucentPassDrawCalls = 0;
        DebugPassDrawCalls = 0;
        TotalRenderTime = 0.0f;
        PickingTime = 0.0f;
        // BasePassTime = 0.0f;
        // SortTime = 0.0f;
    }
};

/**
 * @brief 렌더링 통계 수집 및 관리 싱글톤
 */
class URenderingStatsCollector
{
public:
    static URenderingStatsCollector& GetInstance();

    // 통계 업데이트
    void UpdateFrameStats(const FRenderingStats& InNewStats);
    void BeginFrame();
    void EndFrame();

    // 성능 측정 (미사용 - 나중에 RenderPass 시스템 구현 시 추가 예정)
    // void BeginRenderTimer(const char* PassName);
    // void EndRenderTimer(const char* PassName);

    // 통계 증분 메서드들
    void IncrementDrawCalls() { if (bEnabled) CurrentFrameStats.TotalDrawCalls++; }
    void IncrementMaterialChanges() { if (bEnabled) CurrentFrameStats.MaterialChanges++; }
    void IncrementTextureChanges() { if (bEnabled) CurrentFrameStats.TextureChanges++; }
    void IncrementShaderChanges() { if (bEnabled) CurrentFrameStats.ShaderChanges++; }
    void IncrementBasePassDrawCalls() { if (bEnabled) CurrentFrameStats.BasePassDrawCalls++; }

    void IncrementDepthPrePassDrawCalls()
    {
        if (bEnabled) CurrentFrameStats.DepthPrePassDrawCalls++;
    }

    void IncrementTranslucentPassDrawCalls()
    {
        if (bEnabled) CurrentFrameStats.TranslucentPassDrawCalls++;
    }

    void IncrementDebugPassDrawCalls() { if (bEnabled) CurrentFrameStats.DebugPassDrawCalls++; }

    // 통계 접근
    const FRenderingStats& GetCurrentFrameStats() const { return CurrentFrameStats; }
    const FRenderingStats& GetAverageStats() const { return AverageStats; }
    
    // 피킹 타임 접근
    void UpdatePickingStats(double InPickingTimeMs);
    double GetLastPickingTime() const;
    uint64_t GetNumPickingAttempts() const;
    double GetAccumulatedPickingTime() const;

    // 설정
    void SetEnabled(bool bInEnabled) { bEnabled = bInEnabled; }
    bool IsEnabled() const { return bEnabled; }

    // 통계 리셋
    void ResetStats();

private:
    URenderingStatsCollector() = default;
    ~URenderingStatsCollector() = default;
    URenderingStatsCollector(const URenderingStatsCollector&) = delete;
    URenderingStatsCollector& operator=(const URenderingStatsCollector&) = delete;

    // 평균 계산
    void CalculateAverageStats();

private:
    bool bEnabled = true;

    // 현재 프레임 통계
    FRenderingStats CurrentFrameStats;

    // 평균 통계 (10프레임 평균)
    FRenderingStats AverageStats;
    static const uint32 AVERAGE_FRAME_COUNT = 10;
    FRenderingStats FrameHistory[AVERAGE_FRAME_COUNT];
    uint32 FrameHistoryIndex = 0;
    uint32 ValidFrameCount = 0;

    // 피킹 타임
    //double CurrentPickingTime = 0.0f;
    double   LastPickingTimeMs;        // 가장 마지막 피킹 시간
    uint64 NumPickingAttempts;       // 누적 피킹 시도 횟수
    double   AccumulatedPickingTimeMs; // 누적 피킹 시간

    // 타이머 시스템
    std::chrono::high_resolution_clock::time_point FrameStartTime;

    // 최적화 효과 계산용
    uint32 PreviousUnoptimizedStateChanges = 0;
};
