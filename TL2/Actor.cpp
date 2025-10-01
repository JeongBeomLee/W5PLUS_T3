#include "pch.h"
#include "Actor.h"
#include "ActorComponent.h"
#include "ObjectFactory.h"
#include "ShapeComponent.h"
#include "AABoundingBoxComponent.h"   
#include "MeshComponent.h"
#include "TextRenderComponent.h"
AActor::AActor()
{
    Name = "DefaultActor";
    RootComponent= CreateDefaultSubobject<USceneComponent>(FName("SceneComponent"));
    //CollisionComponent = CreateDefaultSubobject<UAABoundingBoxComponent>(FName("CollisionBox"));
    //UTextRenderComponent* TextComp = NewObject<UTextRenderComponent>();
    //TextComp->SetOwner(this);
   // AddComponent(TextComp);    
}

AActor::~AActor()
{
    //// 1) Delete root: cascades to attached children
    //if (RootComponent)
    //{
    //    ObjectFactory::DeleteObject(RootComponent);
    //    RootComponent = nullptr;
    //}
    // 2) Delete any remaining components not under the root tree (safe: DeleteObject checks GUObjectArray)
    for (UActorComponent* Comp : Components)
    {
        if (Comp)
        {
            ObjectFactory::DeleteObject(Comp);
            Comp = nullptr;
        }
    }
    Components.Empty();
    //TextComp->SetupAttachment(GetRootComponent());
}

void AActor::BeginPlay()
{
    // 소유한 모든 컴포넌트의 BeginPlay 호출
    for (UActorComponent* Component : Components)
    {
        if (Component)
        {
            Component->BeginPlay();
        }
    }
}

void AActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 소유한 모든 컴포넌트의 EndPlay 호출
    for (UActorComponent* Component : Components)
    {
        if (Component)
        {
            Component->EndPlay();
        }
    }
}

void AActor::Tick(float DeltaSeconds)
{
    // 소유한 모든 컴포넌트의 Tick 처리
    for (UActorComponent* Component : Components)
    {
        if (Component && Component->IsComponentTickEnabled())
        {
            Component->TickComponent(DeltaSeconds);
        }
    }
}

void AActor::Destroy()
{
    if (!bCanEverTick) return;
    // Prefer world-managed destruction to remove from world actor list
    if (World)
    {
        // Avoid using 'this' after the call
        World->DestroyActor(this);
        return;
    }
    // Fallback: directly delete the actor via factory
    ObjectFactory::DeleteObject(this);
}

void AActor::SetActorLocation(const FVector& NewLocation)
{
    if (RootComponent)
    {
        RootComponent->SetRelativeLocation(NewLocation);
    }
}

FVector AActor::GetActorLocation() const
{
    return RootComponent ? RootComponent->GetRelativeLocation() : FVector();
}

void AActor::SetActorRotation(const FVector& EulerDegree)
{
    if (RootComponent)
    {
        RootComponent->SetRelativeRotation(FQuat::MakeFromEuler(EulerDegree));
    }
}

void AActor::SetActorRotation(const FQuat& InQuat)
{
    if (RootComponent)
    {
        RootComponent->SetRelativeRotation(InQuat);
    }
}

FQuat AActor::GetActorRotation() const
{
    return RootComponent ? RootComponent->GetRelativeRotation() : FQuat();
}

void AActor::SetActorScale(const FVector& NewScale)
{
    if (RootComponent)
    {
        RootComponent->SetRelativeScale(NewScale);
    }
}

FVector AActor::GetActorScale() const
{
    return RootComponent ? RootComponent->GetRelativeScale() : FVector(1, 1, 1);
}

FMatrix AActor::GetWorldMatrix() const
{
    return RootComponent ? RootComponent->GetWorldMatrix() : FMatrix::Identity();
}

void AActor::SetActorTransform(const FTransform& Transform)
{
    RootComponent->SetRelativeTransform(Transform);
}

const TSet<UActorComponent*>& AActor::GetAllComponents() const
{
    return Components;
}

void AActor::AddComponent(UActorComponent* Component)
{
    if (!Component)
    {
        return;
    }

    USceneComponent* SceneComp = nullptr;
    if (SceneComp = Cast<USceneComponent>(Component))
    {
        if (!RootComponent)
        {
            RootComponent = SceneComp;
        }
        else
        {
            SceneComp->SetupAttachment(RootComponent);
        }
    }
    Components.insert(Component);
    Component->SetOwner(this);
}

// AActor 복제
UObject* AActor::Duplicate()
{
    // 새 액터 생성 (같은 타입으로)
    AActor* NewActor = NewObject<AActor>();
    if (!NewActor)
    {
        return nullptr;
    }

    // 기본 프로퍼티 복사 (간소화된 얕은 복사)
    NewActor->Name = this->Name;
    NewActor->bIsPicked = false; // PIE에서는 선택 해제
    NewActor->bCanEverTick = this->bCanEverTick;
    NewActor->bHiddenInGame = this->bHiddenInGame;
    NewActor->bTickInEditor = this->bTickInEditor;

    // 서브 오브젝트(Components) 복제
    NewActor->DuplicateSubObjects();

    return NewActor;
}

// 서브 오브젝트(Components) 복제
void AActor::DuplicateSubObjects()
{
    // 컴포넌트 복제 및 매핑 테이블 생성
    TSet<UActorComponent*> OriginalComponents = Components;
    Components.clear();

    // 원본 -> 복제본 매핑
    TMap<UActorComponent*, UActorComponent*> ComponentMap;

    for (UActorComponent* OriginalComponent : OriginalComponents)
    {
        if (OriginalComponent)
        {
            UActorComponent* NewComp = Cast<UActorComponent>(OriginalComponent->Duplicate());
            if (NewComp)
            {
                NewComp->SetOwner(this);
                Components.insert(NewComp);
                ComponentMap[OriginalComponent] = NewComp;
            }
        }
    }

    // SceneComponent 계층 구조 복원
    for (const auto& Pair : ComponentMap)
    {
        USceneComponent* OriginalSceneComponent = Cast<USceneComponent>(Pair.first);
        USceneComponent* NewSceneComponent = Cast<USceneComponent>(Pair.second);

        if (OriginalSceneComponent && NewSceneComponent)
        {
            // 원본의 AttachParent가 있으면 복제본도 동일한 계층 구조로 설정
            USceneComponent* OriginalParent = OriginalSceneComponent->GetAttachParent();
            if (OriginalParent && ComponentMap.find(OriginalParent) != ComponentMap.end())
            {
                USceneComponent* NewParent = Cast<USceneComponent>(ComponentMap[OriginalParent]);
                if (NewParent)
                {
                    NewSceneComponent->SetupAttachment(NewParent);
                }
            }
        }
    }

    // RootComponent 설정
    if (RootComponent)
    {
        USceneComponent* OriginalRootComponent = RootComponent;
        if (ComponentMap.find(OriginalRootComponent) != ComponentMap.end())
        {
            RootComponent = Cast<USceneComponent>(ComponentMap[OriginalRootComponent]);
        }
    }
}
