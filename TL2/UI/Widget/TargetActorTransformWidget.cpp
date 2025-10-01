#include "pch.h"
#include "TargetActorTransformWidget.h"
#include "UI/UIManager.h"
#include "ImGui/imgui.h"
#include "Actor.h"
#include "World.h"
#include "Vector.h"
#include "GizmoActor.h"
#include <string>
#include "StaticMeshActor.h"    
#include "StaticMeshComponent.h"
#include "ResourceManager.h"    
#include "SelectionManager.h"

using namespace std;

//// UE_LOG 대체 매크로
//#define UE_LOG(fmt, ...)

UTargetActorTransformWidget::UTargetActorTransformWidget()
	: UWidget("Target Actor Transform Widget")
	, UIManager(&UUIManager::GetInstance())
{

}

UTargetActorTransformWidget::~UTargetActorTransformWidget() = default;

void UTargetActorTransformWidget::OnSelectedActorCleared()
{
	// 즉시 내부 캐시/플래그 정리
	SelectedActor = nullptr;
	SelectedComponent = nullptr;
	CachedActorName.clear();
}

void UTargetActorTransformWidget::Initialize()
{
	// UIManager 참조 확보
	UIManager = &UUIManager::GetInstance();

	// GizmoActor 참조 획득
	GizmoActor = UIManager->GetGizmoActor();

	// 초기 기즈모 스페이스 모드 설정
	if (GizmoActor)
	{
		CurrentGizmoSpace = GizmoActor->GetSpace();
	}

	// Transform 위젯을 UIManager에 등록하여 선택 해제 브로드캐스트를 받을 수 있게 함
	if (UIManager)
	{
		UIManager->RegisterTargetTransformWidget(this);
	}
}

void UTargetActorTransformWidget::Update()
{
	// UIManager를 통해 현재 선택된 액터 가져오기
	AActor* CurrentSelectedActor = USelectionManager::GetInstance().GetSelectedActor();
	SelectedComponent = USelectionManager::GetInstance().GetSelectedComponent();

	if (SelectedActor != CurrentSelectedActor)
	{
		SelectedActor = CurrentSelectedActor;
		// 새로 선택된 액터의 이름 캐시
		if (SelectedActor)
		{
			try
			{
				CachedActorName = SelectedActor->GetName().ToString();
			}
			catch (...)
			{
				CachedActorName = "[Invalid Actor]";
				SelectedActor = nullptr;
			}
		}
		else
		{
			CachedActorName = "";
		}
	}

	// GizmoActor 참조 업데이트
	if (!GizmoActor && UIManager)
	{
		GizmoActor = UIManager->GetGizmoActor();
	}

	// 월드 정보 업데이트 (옵션)
	if (UIManager && UIManager->GetWorld())
	{
		UWorld* World = UIManager->GetWorld();
		WorldActorCount = static_cast<uint32>(World->GetActors().size());
	}
}

void UTargetActorTransformWidget::RenderWidget()
{
	// 월드 정보 표시
	ImGui::Text("World Information");
	ImGui::Text("Actor Count: %u", WorldActorCount);
	ImGui::Separator();

	AGridActor* gridActor = UIManager->GetWorld()->GetGridActor();
	if (gridActor)
	{
		float currentLineSize = gridActor->GetLineSize();
		if (ImGui::DragFloat("Grid Spacing", &currentLineSize, 0.1f, 0.1f, 1000.0f))
		{
			gridActor->SetLineSize(currentLineSize);
			EditorINI["GridSpacing"] = std::to_string(currentLineSize);
		}
	}
	else
	{
		ImGui::Text("GridActor not found in the world.");
	}

	ImGui::Text("Transform Editor");

	SelectedActor = USelectionManager::GetInstance().GetSelectedActor();
	SelectedComponent = USelectionManager::GetInstance().GetSelectedComponent();

	// 기즈모 스페이스 모드 선택
	if (GizmoActor)
	{
		const char* spaceItems[] = { "World", "Local" };
		int currentSpaceIndex = static_cast<int>(CurrentGizmoSpace);

		if (ImGui::Combo("Gizmo Space", &currentSpaceIndex, spaceItems, IM_ARRAYSIZE(spaceItems)))
		{
			CurrentGizmoSpace = static_cast<EGizmoSpace>(currentSpaceIndex);

			GizmoActor->SetSpaceWorldMatrix(CurrentGizmoSpace, USelectionManager::GetInstance().GetSelectedOnlyComponent());
		}
		ImGui::Separator();
	}

	//Detail
	if (SelectedActor)
	{
		// 액터 이름 표시 (캐시된 이름 사용)
		ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Selected: %s",
			CachedActorName.c_str());
		// 선택된 액터 UUID 표시(전역 고유 ID)
		ImGui::Text("UUID: %u", static_cast<unsigned int>(SelectedActor->UUID));
		ImGui::Spacing();

		const TSet<UActorComponent*>& ActorComponents = SelectedActor->GetAllComponents();
		if (SelectedComponent != nullptr)
		{
			SelectedComponent->RenderDetailCommon();
		}
	}

	//// 기즈모 스페이스 빠른 전환 버튼
	//if (GizmoActor)
	//{
	//	ImGui::Separator();
	//	const char* buttonText = CurrentGizmoSpace == EGizmoSpace::World ? 
	//		"Switch to Local" : "Switch to World";
	//	
	//	if (ImGui::Button(buttonText))
	//	{
	//		// 스페이스 모드 전환
	//		CurrentGizmoSpace = (CurrentGizmoSpace == EGizmoSpace::World) ? 
	//			EGizmoSpace::Local : EGizmoSpace::World;
	//		
	//		// 기즈모 액터에 스페이스 설정 적용
	//		GizmoActor->SetSpaceWorldMatrix(CurrentGizmoSpace, SelectedActor);
	//	}
	//	
	//	ImGui::SameLine();
	//	ImGui::Text("Current: %s", 
	//		CurrentGizmoSpace == EGizmoSpace::World ? "World" : "Local");
	//}
//	
//	ImGui::Spacing();
//	ImGui::Separator();

}
