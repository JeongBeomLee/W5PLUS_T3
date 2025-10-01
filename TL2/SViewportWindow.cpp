#include "pch.h"
#include "SViewportWindow.h"
#include "World.h"
#include "ImGui/imgui.h"
#include "SMultiViewportWindow.h"
#include "Globals.h"
extern float CLIENTWIDTH;
extern float CLIENTHEIGHT;
SViewportWindow::SViewportWindow()
{
	ViewportType = EViewportType::Perspective;
	bIsActive = false;
	bIsMouseDown = false;
}

SViewportWindow::~SViewportWindow()
{
	if (Viewport)
	{
		delete Viewport;
		Viewport = nullptr;
	}

	if (ViewportClient)
	{
		delete ViewportClient;
		ViewportClient = nullptr;
	}
}

bool SViewportWindow::Initialize(float StartX, float StartY, float Width, float Height, UWorld* World, ID3D11Device* Device, EViewportType InViewportType)
{
	ViewportType = InViewportType;

	// 이름 설정
	switch (ViewportType)
	{
	case EViewportType::Perspective:       ViewportName = "Perspective"; break;
	case EViewportType::Orthographic_Front: ViewportName = "Front"; break;
	case EViewportType::Orthographic_Left:  ViewportName = "Left"; break;
	case EViewportType::Orthographic_Top:   ViewportName = "Top"; break;
	case EViewportType::Orthographic_Back: ViewportName = "Back"; break;
	case EViewportType::Orthographic_Right:  ViewportName = "Right"; break;
	case EViewportType::Orthographic_Bottom:   ViewportName = "Bottom"; break;
	}

	// FViewport 생성
	Viewport = new FViewport();
	if (!Viewport->Initialize(StartX, StartY, Width, Height, Device))
	{
		delete Viewport;
		Viewport = nullptr;
		return false;
	}

	// FViewportClient 생성
	ViewportClient = new FViewportClient();
	ViewportClient->SetViewportType(ViewportType);
	ViewportClient->SetWorld(World); // 전역 월드 연결 (이미 있다고 가정)

	// 양방향 연결
	Viewport->SetViewportClient(ViewportClient);

	return true;
}

void SViewportWindow::OnRender()
{
	if (!Viewport)
		return;

	Viewport->BeginRenderFrame();
	RenderToolbar();
	if (ViewportClient)
		ViewportClient->Draw(Viewport);

	// PIE 모드 페이드 인/아웃
	RenderPIEOverlay();

	Viewport->EndRenderFrame();
}

void SViewportWindow::OnUpdate(float DeltaSeconds)
{
	if (!Viewport)
		return;

	if (!Viewport) return;

	// PIE 테두리 알파값 업데이트 (페이드 인/아웃)
	const float FadeSpeed = 2.0f;
	if (PIEWorld != nullptr && ViewportClient && ViewportClient->GetWorld()->IsPIEWorld())
	{
		// Fade In
		PIEBorderAlpha += DeltaSeconds * FadeSpeed;
		if (PIEBorderAlpha > 1.0f)
			PIEBorderAlpha = 1.0f;
	}
	else
	{
		// Fade Out
		PIEBorderAlpha -= DeltaSeconds * FadeSpeed;
		if (PIEBorderAlpha < 0.0f)
			PIEBorderAlpha = 0.0f;
	}

	uint32 NewStartX = static_cast<uint32>(Rect.Left);
	uint32 NewStartY = static_cast<uint32>(Rect.Top );
	uint32 NewWidth = static_cast<uint32>(Rect.Right - Rect.Left);
	uint32 NewHeight = static_cast<uint32>(Rect.Bottom - Rect.Top );

	Viewport->Resize(NewStartX, NewStartY, NewWidth, NewHeight);
	ViewportClient->Tick(DeltaSeconds);
}

void SViewportWindow::OnMouseMove(FVector2D MousePos)
{
	if (!Viewport) return;

	// 툴바 영역 아래에서만 마우스 이벤트 처리
	
	
		FVector2D LocalPos = MousePos - FVector2D(Rect.Left, Rect.Top );
		Viewport->ProcessMouseMove((int32)LocalPos.X, (int32)LocalPos.Y);
	
}

void SViewportWindow::OnMouseDown(FVector2D MousePos, uint32 Button)
{
	if (!Viewport) return;

	// 툴바 영역 아래에서만 마우스 이벤트 처리s
		bIsMouseDown = true;
		FVector2D LocalPos = MousePos - FVector2D(Rect.Left, Rect.Top );
		Viewport->ProcessMouseButtonDown((int32)LocalPos.X, (int32)LocalPos.Y, Button);
	
}

void SViewportWindow::OnMouseUp(FVector2D MousePos, uint32 Button)
{
	if (!Viewport) return;

	// 툴바 영역 아래에서만 마우스 이벤트 처리

		bIsMouseDown = false;
		FVector2D LocalPos = MousePos - FVector2D(Rect.Left, Rect.Top );
		Viewport->ProcessMouseButtonUp((int32)LocalPos.X, (int32)LocalPos.Y, Button);
	
}

void SViewportWindow::SetMainViewPort()
{
	Viewport->SetMainViewport();
}

void SViewportWindow::RenderToolbar()
{
	if (!Viewport) return;

	// 툴바 영역 크기
	float toolbarHeight = 30.0f;
	ImVec2 toolbarPos(Rect.Left, Rect.Top);
	ImVec2 toolbarSize(Rect.Right - Rect.Left, toolbarHeight);

	// 툴바 위치 지정
	ImGui::SetNextWindowPos(toolbarPos);
	ImGui::SetNextWindowSize(toolbarSize);

	// 뷰포트별 고유한 윈도우 ID
	char windowId[64];
	sprintf_s(windowId, "ViewportToolbar_%p", this);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

	if (ImGui::Begin(windowId, nullptr, flags))
	{
		// PIE 모드 체크
		bool bIsPIEMode = (PIEWorld != nullptr && ViewportClient && ViewportClient->GetWorld()->IsPIEWorld());

		// PIE 모드일 때 버튼들 비활성화 (Stop 제외)
		if (bIsPIEMode)
		{
			ImGui::BeginDisabled();
		}

		// 뷰포트 모드 선택 콤보박스
		const char* viewportModes[] = {
			"Perspective",
			"Top",
			"Bottom",
			"Front",
			"Left",
			"Right",
			"Back"
		};

		int currentMode = static_cast<int>(ViewportType);
		ImGui::SetNextItemWidth(100);
		if (ImGui::Combo("##ViewportMode", &currentMode, viewportModes, IM_ARRAYSIZE(viewportModes)))
		{
			EViewportType newType = static_cast<EViewportType>(currentMode);
			if (newType != ViewportType)
			{
				ViewportType = newType;

				// ViewportClient 업데이트
				if (ViewportClient)
				{
					ViewportClient->SetViewportType(ViewportType);
					ViewportClient->SetupCameraMode();

				}

				// 뷰포트 이름 업데이트
				switch (ViewportType)
				{
				case EViewportType::Perspective:       ViewportName = "Perspective"; break;
				case EViewportType::Orthographic_Front: ViewportName = "Front"; break;
				case EViewportType::Orthographic_Left:  ViewportName = "Left"; break;
				case EViewportType::Orthographic_Top:   ViewportName = "Top"; break;
				case EViewportType::Orthographic_Back: ViewportName = "Back"; break;
				case EViewportType::Orthographic_Right:  ViewportName = "Right"; break;
				case EViewportType::Orthographic_Bottom:   ViewportName = "Bottom"; break;
				}
			}
		}
		ImGui::SameLine();

		// 뷰포트 이름 표시
		ImGui::Text("%s", ViewportName.ToString().c_str());
		ImGui::SameLine();

		// 버튼들
		if (ImGui::Button("Move")) { /* TODO: 이동 모드 전환 */ }
		ImGui::SameLine();

		if (ImGui::Button("Rotate")) { /* TODO: 회전 모드 전환 */ }
		ImGui::SameLine();

		if (ImGui::Button("Scale")) { /* TODO: 스케일 모드 전환 */ }
		ImGui::SameLine();

		if (ImGui::Button("Reset")) { /* TODO: 카메라 Reset */ }
		ImGui::SameLine();

		const char* viewModes[] = { "Lit", "Unlit", "Wireframe" };
		int currentViewMode = static_cast<int>(ViewportClient-> GetViewModeIndex())-1; // 0=Lit, 1=Unlit, 2=Wireframe -1이유 1부터 시작이여서

		ImGui::SameLine();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2)); // 버튼/콤보 내부 여백 축소
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 0)); // 아이템 간 간격 축소
		ImGui::SetNextItemWidth(80.0f);                                // ✅ 폭 줄이기
		bool changed = ImGui::Combo("##ViewMode", &currentViewMode, viewModes, IM_ARRAYSIZE(viewModes));
		ImGui::PopStyleVar(2);

		if (changed && ViewportClient)
		{
			switch (currentViewMode)
			{
			case 0: ViewportClient->SetViewModeIndex(EViewModeIndex::VMI_Lit); break;
			case 1: ViewportClient->SetViewModeIndex(EViewModeIndex::VMI_Unlit); break;
			case 2: ViewportClient->SetViewModeIndex(EViewModeIndex::VMI_Wireframe); break;
			}
		}

		// PIE 모드 비활성화 종료
		if (bIsPIEMode)
		{
			ImGui::EndDisabled();
		}
		// 🔘 여기 ‘한 번 클릭’ 버튼 추가
		const float btnW = 60.0f;
		const ImVec2 btnSize(btnW, 0.0f);


		// PIE 버튼
		ImGui::SameLine();
		if (PIEWorld == nullptr)
		{
			// Play 버튼 - 진한 녹색
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));       
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.5f, 0.1f, 1.0f)); 

			if (ImGui::Button("Play (PIE)"))
			{
				StartPIE();
			}

			ImGui::PopStyleColor(3);
		}
		else
		{
			// Stop 버튼 - 빨간색
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));        
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f)); 
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));  

			if (ImGui::Button("Stop (PIE)"))
			{
				EndPIE();
			}

			ImGui::PopStyleColor(3);
		}

		ImGui::SameLine();
		float avail = ImGui::GetContentRegionAvail().x;      // 현재 라인에서 남은 가로폭
		if (avail > btnW) {
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - btnW));
		}

		// Switch 버튼도 PIE 모드에서 비활성화
		if (bIsPIEMode)
		{
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Switch##ToThis", btnSize))
		{
			if (GWorld && GWorld->GetMultiViewportWindow())
				GWorld->GetMultiViewportWindow()->SwitchPanel(this);
		}

		if (bIsPIEMode)
		{
			ImGui::EndDisabled();
		}

		//ImGui::PopStyleVar();

	}
	ImGui::End();
}

void SViewportWindow::StartPIE()
{
	if (!GEditor || !ViewportClient || PIEWorld != nullptr)
	{
		return; // 이미 PIE 실행 중
	}

	// Editor 월드 가져오기
	EditorWorld = GEditor->GetEditorWorld();
	if (!EditorWorld)
	{
		return;
	}

	// Editor 월드를 PIE 월드로 복제
	PIEWorld = UWorld::DuplicateWorldForPIE(EditorWorld);
	if (!PIEWorld)
	{
		return;
	}

	// PIE 월드를 WorldContext로 등록
	FWorldContext* PIEContext = GEditor->CreateWorldContext(PIEWorld, EWorldType::PIE);
	if (PIEContext)
	{
		PIEContext->OwningEditorWorld = EditorWorld;
	}

	// Editor 월드의 MultiViewport와 MainViewport를 PIE 월드에도 설정
	PIEWorld->SetMultiViewportWindow(EditorWorld->GetMultiViewportWindow());
	PIEWorld->SetMainViewport(EditorWorld->GetMainViewport());

	// GWorld를 PIE 월드로 전환
	GWorld = PIEWorld;

	// ViewportClient를 PIE 월드로 전환
	ViewportClient->SetWorld(PIEWorld);

	// 현재 View Mode 저장
	SavedViewModeIndex = ViewportClient->GetViewModeIndex();

	// Lit 모드로 강제 전환
	ViewportClient->SetViewModeIndex(EViewModeIndex::VMI_Lit);

	// PIE 월드의 모든 액터 BeginPlay 호출
	PIEWorld->InitializeActorsForPlay();

	// FPS Widget의 GameTime 초기화 (PIE 시작 시)
	UUIManager::GetInstance().ResetFPSWidgetGameTime();

	UE_LOG("PIE Started\n");
}

void SViewportWindow::EndPIE()
{
	if (!GEditor || !PIEWorld)
	{
		return; // PIE 실행 중이 아님
	}

	// ViewportClient를 Editor 월드로 먼저 복원
	EditorWorld = GEditor->GetEditorWorld();
	if (EditorWorld && ViewportClient)
	{
		ViewportClient->SetWorld(EditorWorld);

		// 저장된 View Mode 복구
		ViewportClient->SetViewModeIndex(SavedViewModeIndex);
	}

	// FPS Widget의 GameTime 초기화 (PIE 종료 시)
	UUIManager::GetInstance().ResetFPSWidgetGameTime();

	// PIE 종료 요청 (다음 프레임 시작 시 정리됨)
	GEditor->RequestEndPIE();

	// 로컬 상태 정리
	PIEWorld = nullptr;

	UE_LOG("PIE Stop Requested\n");
}

void SViewportWindow::RenderPIEOverlay()
{
	// 알파값이 0이면 그리지 않음
	if (PIEBorderAlpha <= 0.0f)
		return;
	if (ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId))
		return;

	// 뷰포트 테두리 (초록색)
	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	ImVec2 min(Rect.Left, Rect.Top / 0.35f);
	ImVec2 max(Rect.Right, Rect.Bottom);

	// 초록색 테두리 (두께 4픽셀) - 알파값 적용
	int alpha = static_cast<int>(PIEBorderAlpha * 255.0f);
	ImU32 borderColor = IM_COL32(0, 255, 0, alpha); // 초록색 + 페이드 알파
	drawList->AddRect(min, max, borderColor, 0.0f, 0, 4.0f);
}