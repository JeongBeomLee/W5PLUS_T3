#include "pch.h"
#include "ActorTerminationWidget.h"
#include "../UIManager.h"
#include "../../ImGui/imgui.h"
#include "../../Actor.h"
#include "../../InputManager.h"
#include "../../World.h"
#include "SelectionManager.h"
#include "SceneComponent.h"
#include "StaticMeshComponent.h"
#include "SimpleRotatingMovementComponent.h"
#include "BillboardComponent.h"

//// UE_LOG 대체 매크로
//#define UE_LOG(fmt, ...)

UActorTerminationWidget::UActorTerminationWidget()
	: UWidget("Actor Termination Widget")
	, SelectedActor(nullptr)
	, UIManager(&UUIManager::GetInstance())
{
}

UActorTerminationWidget::~UActorTerminationWidget() = default;

void UActorTerminationWidget::Initialize()
{
	// UIManager 참조 확보
	UIManager = &UUIManager::GetInstance();
	SelectionManager = &USelectionManager::GetInstance();
}

void UActorTerminationWidget::Update()
{
	// UIManager를 통해 현재 선택된 액터 가져오기
	if (UIManager)
	{
		AActor* CurrentSelectedActor = SelectionManager->GetSelectedActor();

		// Update Current Selected Actor
		if (SelectedActor != CurrentSelectedActor)
		{
			SelectedActor = CurrentSelectedActor;
			// 새로 선택된 액터의 이름을 안전하게 캐시
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
	}
}
void UActorTerminationWidget::AddComponentAtSelectedActor(UActorComponent* NewComponent)
{
	if (SelectedActor != nullptr)
	{
		USceneComponent* SceneComponent = Cast<USceneComponent>(NewComponent);
		if (SceneComponent)
		{
			if (SelectedComponent != nullptr)
			{
				//해당 로직을 둘 곳을 몰라서 일단 여기둠
				SceneComponent->SetupAttachment(SelectedComponent);
				SelectedComponent->GetOwner()->AddComponent(SceneComponent);
			}
			else
			{
				//해당 로직을 둘 곳을 몰라서 일단 여기둠
				SelectedActor->AddComponent(SceneComponent);
			}
		}
		else
		{
			//액터로 바로 들어간다.
			SelectedActor->AddComponent(NewComponent);
		}
		
	}
	else
	{
		UE_LOG("Selected Actor is Null");
	}
}
void UActorTerminationWidget::RenderWidget()
{
	auto& InputManager = UInputManager::GetInstance();
	SelectedActor = SelectionManager->GetSelectedActor();
	SelectedComponent = SelectionManager->GetSelectedComponent();
	if (ImGui::Button("Delete Actor") || InputManager.IsKeyPressed(VK_DELETE))
	{
		if (SelectedActor != nullptr)
		{
			if (SelectedComponent != nullptr)
			{
				DeleteSelectedComponent();
			}
			else
			{
				DeleteSelectedActor();
			}
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Add Component"))
	{
		ImGui::OpenPopup("ComponentType");
	}
	if (ImGui::BeginPopup("ComponentType"))
	{
		if(ImGui::Selectable("RotatingMovement"))
		{
			AddComponentAtSelectedActor(NewObject<USimpleRotatingMovementComponent>());
		}
		if (ImGui::Selectable("StaticMesh"))
		{
			UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>();
			AddComponentAtSelectedActor(StaticMeshComponent);
			StaticMeshComponent->SetStaticMesh("Data/Cube.obj");
		}
		if (ImGui::Selectable("Billboard"))
		{
			UBillboardComponent* NewBillboard = NewObject<UBillboardComponent>();
			AddComponentAtSelectedActor(NewBillboard);
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	//if (ImGui::Button("Remove Component"))
	//{
	//	if (SelectedActor != nullptr && SelectedComponent != nullptr)
	//	{	
	//		//해당 로직을 둘 곳을 몰라서 일단 여기둠
	//		UStaticMeshComponent* StaticComponent = NewObject<UStaticMeshComponent>();
	//		StaticComponent->SetupAttachment(SelectedComponent);
	//		SelectedComponent->GetOwner()->AddComponent(StaticComponent);
	//	}
	//	else
	//	{
	//		UE_LOG("Selected Component is Null");
	//	}
	//}
	ImGui::Separator();
}

/**
 * @brief Selected Actor 삭제 함수
 */
void UActorTerminationWidget::DeleteSelectedActor()
{
	AActor* DeleteActor = SelectionManager->GetSelectedActor();
	// 기즈모가 이 액터를 타겟으로 잡고 있다면 해제
	if (AGizmoActor* Gizmo = UIManager->GetGizmoActor())
	{
		if (Gizmo->GetTargetActor()->GetOwner() == DeleteActor)
		{
			Gizmo->SetTargetComponent(nullptr);
		}
	}

	UWorld* World = UIManager->GetWorld();
	World->DestroyActor(DeleteActor);
}
void UActorTerminationWidget::DeleteSelectedComponent()
{

}
