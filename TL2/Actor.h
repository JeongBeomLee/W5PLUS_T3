#pragma once
#include "Object.h"
#include "Vector.h"

class UWorld;
class UActorComponent;
class UAABoundingBoxComponent;
class UShapeComponent;

class AActor : public UObject
{
public:
    DECLARE_CLASS(AActor, UObject)
    AActor(); 

protected:
    ~AActor() override;

public:
    virtual void BeginPlay();
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
    virtual void Tick(float DeltaSeconds);
    virtual void Destroy();

    // 복제 시스템
    virtual UObject* Duplicate() override;
    virtual void DuplicateSubObjects() override;

    // ───────────────
    // Transform API
    // ───────────────
    void SetActorTransform(const FTransform& Transform);

    void SetActorLocation(const FVector& NewLocation);
    FVector GetActorLocation() const;

    void SetActorRotation(const FVector& EulerDegree);
    void SetActorRotation(const FQuat& InQuat);
    FQuat GetActorRotation() const;

    void SetActorScale(const FVector& NewScale);
    FVector GetActorScale() const;

    FMatrix GetWorldMatrix() const;

    FVector GetActorForward() { return RootComponent->GetForward(); }
    FVector GetActorRight() { return RootComponent->GetRight(); }
    FVector GetActorUp() { return RootComponent->GetUp(); }

    void SetWorld(UWorld* InWorld) { World = InWorld; }
    UWorld* GetWorld() const { return World; }

    USceneComponent* GetRootComponent() { return RootComponent; }

    void SetIsPicked(bool picked) { bIsPicked = picked; }
    bool GetIsPicked() { return bIsPicked; }

    //-----------------------------
    //----------Getter------------
    const TSet<UActorComponent*>& GetComponents() const;

    void SetName(const FString& InName) { Name = InName; }
    const FName& GetName() { return Name; }

    template<typename T>
    T* CreateDefaultSubobject(const FName& SubobjectName)
    {
        // NewObject를 통해 생성
        T* Comp = ObjectFactory::NewObject<T>();
        Comp->SetOwner(this);
       // Comp->SetName(SubobjectName);  //나중에 추가 구현
        AddComponent(Comp);
        return Comp;
    }

public:
    FName Name;
    USceneComponent* RootComponent = nullptr;
    UAABoundingBoxComponent* CollisionComponent = nullptr;

    UWorld* World = nullptr;
    
    // Visibility properties
    void SetActorHiddenInGame(bool bNewHidden) { bHiddenInGame = bNewHidden; }
    bool GetActorHiddenInGame() const { return bHiddenInGame; }
    bool IsActorVisible() const { return !bHiddenInGame; }

    // Tick In Editor
    void SetActorTickInEditorEnabled(bool bEnabled) { bTickInEditor = bEnabled; }
    bool IsActorTickInEditorEnabled() const { return bTickInEditor; }

    // Tick Enabled Check
    bool IsActorTickEnabled() const { return bCanEverTick; }

    // Tick 조건 헬퍼 함수들
    bool ShouldTickInEditor() const { return bTickInEditor; }
    bool CanTickInPlayMode() const { return bCanEverTick && !bHiddenInGame; }

    void AddComponent(UActorComponent* Component);

protected:
    TSet<UActorComponent*> Components;
    bool bIsPicked = false;
    bool bCanEverTick = true;
    bool bHiddenInGame = false;
    bool bTickInEditor = false;
};
