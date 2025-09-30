#pragma once

#include <d3d11.h>
#include <dxgi.h>

class UStatsOverlayD2D
{
public:
    static UStatsOverlayD2D& Get();

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* swapChain);
    void Draw();

    void SetShowFPS(bool b); 
    void SetShowMemory(bool b);
    void SetShowRenderStats(bool b);
    void ToggleFPS();
    void ToggleMemory();
    void ToggleRenderStats();
    bool IsFPSVisible() const { return bShowFPS; }
    bool IsMemoryVisible() const { return bShowMemory; }
    bool IsRenderStatsVisible() const { return bShowRenderStats; }
    
    // 렌더링 통계 업데이트
    void UpdateRenderingStats(uint32 InDrawCalls, uint32 InMaterialChanges,
                              uint32 InTextureChanges, uint32 InShaderChanges);

private:
    UStatsOverlayD2D() = default;
    ~UStatsOverlayD2D() = default;
    UStatsOverlayD2D(const UStatsOverlayD2D&) = delete;
    UStatsOverlayD2D& operator=(const UStatsOverlayD2D&) = delete;

    void EnsureInitialized();
    void ReleaseD2DTarget();

private:
    bool bInitialized = false;
    bool bShowFPS = false;
    bool bShowMemory = false;
    bool bShowRenderStats = false;
    
    // 렌더링 통계 데이터
    uint32 CurrentDrawCalls = 0;
    uint32 CurrentMaterialChanges = 0;
    uint32 CurrentTextureChanges = 0;
    uint32 CurrentShaderChanges = 0;
    
    // 10프레임 평균
    static const int STATS_HISTORY_SIZE = 10;
    uint32 DrawCallsHistory[STATS_HISTORY_SIZE] = {0};
    uint32 MaterialChangesHistory[STATS_HISTORY_SIZE] = {0};
    uint32 TextureChangesHistory[STATS_HISTORY_SIZE] = {0};
    uint32 ShaderChangesHistory[STATS_HISTORY_SIZE] = {0};
    int StatsHistoryIndex = 0;

    ID3D11Device* D3DDevice = nullptr;
    ID3D11DeviceContext* D3DContext = nullptr;
    IDXGISwapChain* SwapChain = nullptr;
};
