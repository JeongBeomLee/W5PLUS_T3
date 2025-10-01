#pragma once
#include "Widget.h"
#include "../../Vector.h"
#include "../../UEContainer.h"

class UUIManager;
class UWorld;
class AActor;
class USelectionManager;
class USceneComponent;

/**
 * SceneManagerWidget
 * - Unreal Engine style hierarchical object browser
 * - Shows all actors in the world in a tree view
 * - Supports selection, visibility toggle, hierarchy management
 * - Syncs with 3D viewport selection
 */
class USceneManagerWidget : public UWidget
{
public:
    DECLARE_CLASS(USceneManagerWidget, UWidget)

    void Initialize() override;
    void Update() override;
    void RenderWidget() override;

    // Special Member Functions
    USceneManagerWidget();
    ~USceneManagerWidget() override;

private:
    void RenderComponent(USceneComponent* Component);

private:
    UUIManager* UIManager = nullptr;
    USelectionManager* SelectionManager = nullptr;
    AActor* SelectedActor = nullptr;
    USceneComponent* SelectedComponent = nullptr;

    // Helper Methods
    UWorld* GetCurrentWorld() const;



};