#include "pch.h"
#include "StatsOverlayD2D.h"
#include "RenderingStats.h"

#include <d2d1_1.h>
#include <dwrite.h>
#include <dxgi1_2.h>
#include "UI/UIManager.h"
#include "MemoryManager.h"
#include <psapi.h>    // GetProcessMemoryInfo 정의됨
#pragma comment(lib, "psapi.lib")  // 링커에 추가
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

static inline void SafeRelease(IUnknown* p) { if (p) p->Release(); }

UStatsOverlayD2D& UStatsOverlayD2D::Get()
{
    static UStatsOverlayD2D Instance;
    return Instance;
}

void UStatsOverlayD2D::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* swapChain)
{
    D3DDevice = device;
    D3DContext = context;
    SwapChain = swapChain;
    bInitialized = (D3DDevice && D3DContext && SwapChain);
}

void UStatsOverlayD2D::EnsureInitialized()
{
}

void UStatsOverlayD2D::ReleaseD2DTarget()
{
}

static void DrawTextBlock(
    ID2D1DeviceContext* d2dCtx,
    IDWriteFactory* dwrite,
    const wchar_t* text,
    const D2D1_RECT_F& rect,
    float fontSize,
    D2D1::ColorF bgColor,
    D2D1::ColorF textColor)
{
    if (!d2dCtx || !dwrite || !text) return;

    ID2D1SolidColorBrush* brushFill = nullptr;
    d2dCtx->CreateSolidColorBrush(bgColor, &brushFill);

    ID2D1SolidColorBrush* brushText = nullptr;
    d2dCtx->CreateSolidColorBrush(textColor, &brushText);

    d2dCtx->FillRectangle(rect, brushFill);

    IDWriteTextFormat* format = nullptr;
    dwrite->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"en-us",
        &format);

    if (format)
    {
        format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        d2dCtx->DrawTextW(
            text,
            static_cast<UINT32>(wcslen(text)),
            format,
            rect,
            brushText);
        format->Release();
    }

    SafeRelease(brushText);
    SafeRelease(brushFill);
}

void UStatsOverlayD2D::UpdateRenderingStats(uint32 InDrawCalls, uint32 InMaterialChanges, uint32 InTextureChanges, uint32 InShaderChanges)
{
    // 현재 데이터 업데이트
    CurrentDrawCalls = InDrawCalls;
    CurrentMaterialChanges = InMaterialChanges;
    CurrentTextureChanges = InTextureChanges;
    CurrentShaderChanges = InShaderChanges;
    
    // 히스토리에 추가
    DrawCallsHistory[StatsHistoryIndex] = InDrawCalls;
    MaterialChangesHistory[StatsHistoryIndex] = InMaterialChanges;
    TextureChangesHistory[StatsHistoryIndex] = InTextureChanges;
    ShaderChangesHistory[StatsHistoryIndex] = InShaderChanges;
    StatsHistoryIndex = (StatsHistoryIndex + 1) % STATS_HISTORY_SIZE;
}

void UStatsOverlayD2D::Draw()
{
    if (!bInitialized || (!bShowFPS && !bShowMemory && !bShowRenderStats) || !SwapChain)
        return;

    ID2D1Factory1* d2dFactory = nullptr;
    D2D1_FACTORY_OPTIONS opts{};
#ifdef _DEBUG
    opts.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &opts, (void**)&d2dFactory)))
        return;

    IDXGISurface* surface = nullptr;
    if (FAILED(SwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&surface)))
    {
        SafeRelease(d2dFactory);
        return;
    }

    IDXGIDevice* dxgiDevice = nullptr;
    if (FAILED(D3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice)))
    {
        SafeRelease(surface);
        SafeRelease(d2dFactory);
        return;
    }

    ID2D1Device* d2dDevice = nullptr;
    if (FAILED(d2dFactory->CreateDevice(dxgiDevice, &d2dDevice)))
    {
        SafeRelease(dxgiDevice);
        SafeRelease(surface);
        SafeRelease(d2dFactory);
        return;
    }

    ID2D1DeviceContext* d2dCtx = nullptr;
    if (FAILED(d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dCtx)))
    {
        SafeRelease(d2dDevice);
        SafeRelease(dxgiDevice);
        SafeRelease(surface);
        SafeRelease(d2dFactory);
        return;
    }

    IDWriteFactory* dwrite = nullptr;
    if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&dwrite)))
    {
        SafeRelease(d2dCtx);
        SafeRelease(d2dDevice);
        SafeRelease(dxgiDevice);
        SafeRelease(surface);
        SafeRelease(d2dFactory);
        return;
    }

    D2D1_BITMAP_PROPERTIES1 bmpProps = {};
    bmpProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    bmpProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    bmpProps.dpiX = 96.0f;
    bmpProps.dpiY = 96.0f;
    bmpProps.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

    ID2D1Bitmap1* targetBmp = nullptr;
    if (FAILED(d2dCtx->CreateBitmapFromDxgiSurface(surface, &bmpProps, &targetBmp)))
    {
        SafeRelease(dwrite);
        SafeRelease(d2dCtx);
        SafeRelease(d2dDevice);
        SafeRelease(dxgiDevice);
        SafeRelease(surface);
        SafeRelease(d2dFactory);
        return;
    }

    d2dCtx->SetTarget(targetBmp);

    d2dCtx->BeginDraw();
    const float margin = 12.0f;
    const float panelWidth = 200.0f;
    const float panelHeight = 48.0f;
    float nextY = margin;

    if (bShowFPS)
    {
        float dt = UUIManager::GetInstance().GetDeltaTime();
        float fps = dt > 0.0f ? (1.0f / dt) : 0.0f;
        float ms = dt * 1000.0f;

        

        wchar_t buf[256];
        swprintf_s(buf, L"FPS: %.1f\nFrame time: %.2f ms",fps,ms);

        D2D1_RECT_F rc = D2D1::RectF(margin, nextY, margin + panelWidth, nextY + panelHeight);
        DrawTextBlock(
            d2dCtx, dwrite, buf, rc, 16.0f,
            D2D1::ColorF(0, 0, 0, 0.6f),
            D2D1::ColorF(D2D1::ColorF::Yellow));

        nextY += panelHeight + 8.0f;
    }
    {
        wchar_t buf[256];
        const URenderingStatsCollector& Instance = URenderingStatsCollector::GetInstance();
        double LastPickingTime = Instance.GetLastPickingTime();
        uint64 NumAttempts = Instance.GetNumPickingAttempts();
        double AccumulatedTime = Instance.GetAccumulatedPickingTime();

        swprintf_s(buf, L"Picking Time %f ms : Num Attempts %llu : Accumulated Time %f ms",
            LastPickingTime,
            NumAttempts,
            AccumulatedTime);

        D2D1_RECT_F rc = D2D1::RectF(margin, nextY, margin + panelWidth * 3.0f, nextY + panelHeight / 2.0f);

        // 4. 헬퍼 함수를 사용해 텍스트를 그립니다.
        DrawTextBlock(
            d2dCtx, dwrite, buf, rc, 16.0f,
            D2D1::ColorF(0, 0, 0, 0.6f),
            D2D1::ColorF(D2D1::ColorF::Green)
        );
        nextY += panelHeight + 4.0f;
    }
    if (bShowMemory)
    {
        // 1) 커스텀 메모리 매니저
        double mb = static_cast<double>(CMemoryManager::TotalAllocationBytes) / (1024.0 * 1024.0);

        // 2) 전체 시스템 메모리
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(memInfo);
        GlobalMemoryStatusEx(&memInfo);
        double totalSysMB = static_cast<double>(memInfo.ullTotalPhys) / (1024.0 * 1024.0);
        double availSysMB = static_cast<double>(memInfo.ullAvailPhys) / (1024.0 * 1024.0);

        // 3) 현재 프로세스 메모리
        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
            sizeof(pmc));
        double workingSetMB = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
        double privateMB = static_cast<double>(pmc.PrivateUsage) / (1024.0 * 1024.0);

        // 버퍼 작성
        wchar_t buf[256];
        swprintf_s(buf,
            L"Custom Alloc: %.1f MB\nAllocs: %u\nProcess WS: %.1f MB\nProcess Private: %.1f MB\nSystem: %.1f MB / Free: %.1f MB",
            mb, CMemoryManager::TotalAllocationCount,
            workingSetMB, privateMB,
            totalSysMB, availSysMB);

        D2D1_RECT_F rc = D2D1::RectF(margin, nextY, margin + panelWidth, nextY + panelHeight * 3);
        DrawTextBlock(
            d2dCtx, dwrite, buf, rc, 16.0f,
            D2D1::ColorF(0, 0, 0, 0.6f),
            D2D1::ColorF(D2D1::ColorF::LightGreen));
            
        nextY += panelHeight + 100.0f;
    }
    
    if (bShowRenderStats)
    {
        uint32 AvgDrawCalls = 0, AvgMaterialChanges = 0, AvgTextureChanges = 0, AvgShaderChanges = 0;
        
        for (int i = 0; i < STATS_HISTORY_SIZE; ++i)
        {
            AvgDrawCalls += DrawCallsHistory[i];
            AvgMaterialChanges += MaterialChangesHistory[i];
            AvgTextureChanges += TextureChangesHistory[i];
            AvgShaderChanges += ShaderChangesHistory[i];
        }
        AvgDrawCalls /= STATS_HISTORY_SIZE;
        AvgMaterialChanges /= STATS_HISTORY_SIZE;
        AvgTextureChanges /= STATS_HISTORY_SIZE;
        AvgShaderChanges /= STATS_HISTORY_SIZE;
        
        wchar_t Buffer[256];
        swprintf_s(Buffer, L"DrawCalls: %u\nMaterials: %u\nTextures: %u\nShaders: %u", 
                  AvgDrawCalls, AvgMaterialChanges, AvgTextureChanges, AvgShaderChanges);

        D2D1_RECT_F rc = D2D1::RectF(margin, nextY, margin + panelWidth, nextY + panelHeight * 1.5f);
        DrawTextBlock(
            d2dCtx, dwrite, Buffer, rc, 14.0f,
            D2D1::ColorF(0, 0, 0, 0.6f),
            D2D1::ColorF(D2D1::ColorF::Cyan));
    }

   

    d2dCtx->EndDraw();
    d2dCtx->SetTarget(nullptr);

    SafeRelease(targetBmp);
    SafeRelease(dwrite);
    SafeRelease(d2dCtx);
    SafeRelease(d2dDevice);
    SafeRelease(dxgiDevice);
    SafeRelease(surface);
    SafeRelease(d2dFactory);
}

void UStatsOverlayD2D::SetShowFPS(bool b)
{
    bShowFPS = b;
}

void UStatsOverlayD2D::SetShowMemory(bool b)
{
    bShowMemory = b;
}

void UStatsOverlayD2D::SetShowRenderStats(bool b)
{
    bShowRenderStats = b;
}

void UStatsOverlayD2D::ToggleFPS()
{
    bShowFPS = !bShowFPS;
}

void UStatsOverlayD2D::ToggleMemory()
{
    bShowMemory = !bShowMemory;
}

void UStatsOverlayD2D::ToggleRenderStats()
{
    bShowRenderStats = !bShowRenderStats;
}
