#include "pch.h"
#include "SceneManagerWidget.h"
#include "../UIManager.h"
#include "../../ImGui/imgui.h"
#include "../../World.h"
#include "../../CameraActor.h"
#include "../../CameraComponent.h"
#include "../../Actor.h"
#include "../../StaticMeshActor.h"
#include "../../SelectionManager.h"
#include <algorithm>
#include <string>

//// UE_LOG 대체 매크로
//#define UE_LOG(fmt, ...)

USceneManagerWidget::USceneManagerWidget()
    : UWidget("Scene Manager")
    , UIManager(&UUIManager::GetInstance())
    , SelectionManager(&USelectionManager::GetInstance())
{
}

USceneManagerWidget::~USceneManagerWidget()
{

}

void USceneManagerWidget::Initialize()
{
    UIManager = &UUIManager::GetInstance();
    SelectionManager = &USelectionManager::GetInstance();
}

void USceneManagerWidget::Update()
{

}

void USceneManagerWidget::RenderWidget()
{
    ImGui::Text("Scene Manager");
    ImGui::Spacing();

    // Toolbar
    //RenderToolbar();
    ImGui::Separator();

    // World status
    UWorld* World = GetCurrentWorld();
    if (!World)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "No World Available");
        return;
    }

    ImGui::Text("Objects: %zu", World->GetActors().size());
    ImGui::Separator();

    // Actor tree view
    ImGui::BeginChild("ActorTreeView", ImVec2(0, 240), true);

    SelectedActor = SelectionManager->GetSelectedActor();
    SelectedComponent = SelectionManager->GetSelectedComponent();
    const TArray<AActor*>& Actors = World->GetActors();
    for (AActor* Actor : Actors)
    {
        if (ImGui::TreeNodeEx(Actor->GetName().ToString().c_str(),
            SelectedComponent == nullptr && SelectedActor == Actor ? ImGuiTreeNodeFlags_Selected : 0))
        {
            if (ImGui::IsItemClicked())
            {
                SelectionManager->SelectActor(Actor);
            }
            RenderComponent(Actor->GetRootComponent());
            ImGui::TreePop();
        }
    }

    ImGui::EndChild();
    // Status bar
    ImGui::Separator();
}

UWorld* USceneManagerWidget::GetCurrentWorld() const
{
    if (!UIManager)
        return nullptr;
    return UIManager->GetWorld();
}





void USceneManagerWidget::RenderComponent(USceneComponent* Component)
{
    const TArray<USceneComponent*>& ChildComponents = Component->GetAttachChildren();
    if (ChildComponents.size() > 0)
    {
        if (ImGui::TreeNodeEx(Component->GetName().c_str(),
            SelectedComponent != nullptr && SelectedComponent == Component ? ImGuiTreeNodeFlags_Selected : 0))
        {
            if (ImGui::IsItemClicked())
            {
                SelectionManager->SelectActor(Component->GetOwner());
                SelectionManager->SelectComponent(Component);
            }

            for (USceneComponent* Child : ChildComponents)
            {
                RenderComponent(Child);
            }
            ImGui::TreePop();
        }
    }
    else
    {
        if (ImGui::Selectable(Component->GetName().c_str(), SelectedComponent != nullptr && SelectedComponent == Component))
        {
            SelectionManager->SelectActor(Component->GetOwner());
            SelectionManager->SelectComponent(Component);
        }
    }
}