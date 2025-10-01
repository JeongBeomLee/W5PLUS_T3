#include "pch.h"
#include "SelectionManager.h"
#include "Actor.h"

USelectionManager& USelectionManager::GetInstance()
{
    static USelectionManager* Instance = nullptr;
    if (Instance == nullptr)
    {
        Instance = NewObject<USelectionManager>();
    }
    return *Instance;
}

void USelectionManager::SelectActor(AActor* Actor)
{
    if (!Actor) return;

    SelectedActor = Actor;
    SelectedComponent = nullptr;
}
void USelectionManager::SelectComponent(USceneComponent* Component)
{
    if (!Component)
    {
        return;
    }
    SelectedComponent = Component;
    SelectedActor = Component->GetOwner();
}
void USelectionManager::DeselectActor(AActor* Actor)
{
    if (!Actor) return;

    if (Actor == SelectedActor)
    {
        SelectedActor = nullptr;
        SelectedComponent = nullptr;
    }
}
void USelectionManager::DeselectComponent(USceneComponent* Component)
{
    if (!Component) return;

    if (Component == SelectedComponent)
    {
        SelectedComponent = nullptr;
    }
}
USceneComponent* USelectionManager::GetSelectedOnlyComponent() const
{
    if (SelectedActor == nullptr)
    {
        return nullptr;
    }
    else
    {
        if (SelectedComponent == nullptr)
        {
            return SelectedActor->GetRootComponent();
        }
        else
        {
            return SelectedComponent;
        }
    }
}


void USelectionManager::ClearSelection()
{
    SelectedActor = nullptr;
    SelectedComponent = nullptr;
}


USelectionManager::USelectionManager()
{

}

USelectionManager::~USelectionManager()
{
    // No-op: instances are destroyed by ObjectFactory::DeleteAll
}
