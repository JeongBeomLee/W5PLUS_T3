#include "pch.h"
#include "ImGuiHelper.h"
#include "../../ImGui/imgui.h"
#include "../../ImGui/imgui_impl_dx11.h"
#include "../../ImGui/imgui_impl_win32.h"
#include "../../Renderer.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam);

UImGuiHelper::UImGuiHelper() = default;

UImGuiHelper::~UImGuiHelper()
{
	Release();
}

static std::string WideToUtf8(const std::wstring& w)
{
	if (w.empty()) return {};
	int len = ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
	std::string out(len, 0);
	::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), out.data(), len, nullptr, nullptr);
	return out;
}

/**
 * @brief ImGui 초기화 함수
 */
[[DEPRECATED]]void UImGuiHelper::Initialize(HWND InWindowHandle)
{
	if (bIsInitialized)
	{
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(InWindowHandle);

	ImGuiIO& IO = ImGui::GetIO();
	
	IO.Fonts->AddFontDefault();
	

	// Note: This overload needs device and context to be passed explicitly
	// Use the new Initialize overload instead
	bIsInitialized = true;
}

/**
 * @brief ImGui 초기화 함수 (디바이스와 컨텍스트 포함)
 */
void UImGuiHelper::Initialize(HWND InWindowHandle, ID3D11Device* InDevice, ID3D11DeviceContext* InDeviceContext)
{
	if (bIsInitialized)
	{
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(InWindowHandle);

	ImGuiIO& IO = ImGui::GetIO();
	// Use Utf8 path
	std::wstring fontPathW = L"Pretendard-Regular.otf";
	const std::string fontPathUtf8 = WideToUtf8(fontPathW);

	ImFontConfig cfg;
	cfg.OversampleH = cfg.OversampleV = 1;
	cfg.PixelSnapH = true;
	cfg.FontDataOwnedByAtlas = true;

	const ImWchar* ranges = IO.Fonts->GetGlyphRangesKorean();
	float sizePx = 18.0f;

	ImFont* font = IO.Fonts->AddFontFromFileTTF(
		fontPathUtf8.c_str(),  // ★ UTF-8 경로 전달
		sizePx,
		&cfg,
		ranges
	);

	if (!font) {
		// 실패 시 기본 내장 폰트로 폴백 (디버그 로그 추천)
		IO.Fonts->AddFontDefault();
	}

	// Initialize ImGui DirectX11 backend with provided device and context
	ImGui_ImplDX11_Init(InDevice, InDeviceContext);

	bIsInitialized = true;
}

/**
 * @brief ImGui 자원 해제 함수
 */
void UImGuiHelper::Release()
{
	if (!bIsInitialized)
	{
		return;
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	bIsInitialized = false;
}

/**
 * @brief ImGui 새 프레임 시작
 */
void UImGuiHelper::BeginFrame() const
{
	if (!bIsInitialized)
	{
		return;
	}

	// Get New Frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

/**
 * @brief ImGui 렌더링 종료 및 출력
 */
void UImGuiHelper::EndFrame() const
{
	if (!bIsInitialized)
	{
		return;
	}

	// Render ImGui
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

/**
 * @brief WndProc Handler 래핑 함수
 * @return ImGui 자체 함수 반환
 */
LRESULT UImGuiHelper::WndProcHandler(HWND hWnd, uint32 msg, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}
