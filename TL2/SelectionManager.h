#pragma once
#include "Object.h"
#include "UEContainer.h"
#include "Vector.h"

// Forward Declarations
class AActor;

/**
 * SelectionManager
 * - 액터 선택 상태를 관리하는 싱글톤 클래스
 * UGameInstanceSubsystem 써야함
 */
class USelectionManager : public UObject
{
public:
    DECLARE_CLASS(USelectionManager, UObject)
    static USelectionManager& GetInstance();

    /** === 선택 관리 === */
    void SelectActor(AActor* Actor);
    void DeselectActor(AActor* Actor);
    void SelectComponent(USceneComponent* Component);
    void DeselectComponent(USceneComponent* Component);;

    void ClearSelection();

    /** === 선택된 액터 접근 === */
    AActor* GetSelectedActor() const { return SelectedActor; }
    USceneComponent* GetSelectedComponent() const { return SelectedComponent; }
    //액터선택 = 루트컴포넌트 리턴, 컴포넌트 선택 = 컴포넌트 리턴
    USceneComponent* GetSelectedOnlyComponent() const;

    bool HasSelection() const { return SelectedActor != nullptr; }

public:
    USelectionManager();
protected:
    ~USelectionManager() override;

    // 복사 금지
    USelectionManager(const USelectionManager&) = delete;
    USelectionManager& operator=(const USelectionManager&) = delete;

    /** === 선택된 액터들 === */
    AActor* SelectedActor;
    USceneComponent* SelectedComponent;
};
