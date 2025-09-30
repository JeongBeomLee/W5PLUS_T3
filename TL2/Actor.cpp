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

// ───────────────
// Transform API
// ───────────────
void AActor::SetActorTransform(const FTransform& NewTransform)
{
    if (RootComponent)
    {
        RootComponent->SetWorldTransform(NewTransform);
    }
}


FTransform AActor::GetActorTransform() const
{
    return RootComponent ? RootComponent->GetWorldTransform() : FTransform();
}

void AActor::SetActorLocation(const FVector& NewLocation)
{
    if (RootComponent)
    {
        RootComponent->SetWorldLocation(NewLocation);
    }
}

FVector AActor::GetActorLocation() const
{
    return RootComponent ? RootComponent->GetWorldLocation() : FVector();
}

void AActor::SetActorRotation(const FVector& EulerDegree)
{
    if (RootComponent)
    {
        RootComponent->SetWorldRotation(FQuat::MakeFromEuler(EulerDegree));
    }
}

void AActor::SetActorRotation(const FQuat& InQuat)
{
    if (RootComponent)
    {
        RootComponent->SetWorldRotation(InQuat);
    }
}

FQuat AActor::GetActorRotation() const
{
    return RootComponent ? RootComponent->GetWorldRotation() : FQuat();
}

void AActor::SetActorScale(const FVector& NewScale)
{
    if (RootComponent)
    {
        RootComponent->SetWorldScale(NewScale);
    }
}

FVector AActor::GetActorScale() const
{
    return RootComponent ? RootComponent->GetWorldScale() : FVector(1, 1, 1);
}

FMatrix AActor::GetWorldMatrix() const
{
    return RootComponent ? RootComponent->GetWorldMatrix() : FMatrix::Identity();
}

void AActor::AddActorWorldRotation(const FQuat& DeltaRotation)
{
    if (RootComponent)
    {
        RootComponent->AddWorldRotation(DeltaRotation);
    }
}

void AActor::AddActorWorldRotation(const FVector& DeltaEuler)
{
    /* if (RootComponent)
     {
         FQuat DeltaQuat = FQuat::FromEuler(DeltaEuler.X, DeltaEuler.Y, DeltaEuler.Z);
         RootComponent->AddWorldRotation(DeltaQuat);
     }*/
}

void AActor::AddActorWorldLocation(const FVector& DeltaLocation)
{
    if (RootComponent)
    {
        RootComponent->AddWorldOffset(DeltaLocation);
    }
}

void AActor::AddActorLocalRotation(const FVector& DeltaEuler)
{
    /*  if (RootComponent)
      {
          FQuat DeltaQuat = FQuat::FromEuler(DeltaEuler.X, DeltaEuler.Y, DeltaEuler.Z);
          RootComponent->AddLocalRotation(DeltaQuat);
      }*/
}

void AActor::AddActorLocalRotation(const FQuat& DeltaRotation)
{
    if (RootComponent)
    {
        RootComponent->AddLocalRotation(DeltaRotation);
    }
}

void AActor::AddActorLocalLocation(const FVector& DeltaLocation)
{
    if (RootComponent)
    {
        RootComponent->AddLocalOffset(DeltaLocation);
    }
}

const TSet<UActorComponent*>& AActor::GetComponents() const
{
    return Components;
}

void AActor::AddComponent(UActorComponent* Component)
{
    if (!Component)
    {
        return;
    }

    Components.insert(Component);
    if (!RootComponent)
    {
        RootComponent = Cast<USceneComponent>(Component);
        //Component->SetupAttachment(RootComponent);
    }
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

    // Transform 복사
    if (this->RootComponent)
    {
        NewActor->SetActorTransform(this->GetActorTransform());
    }

    // 서브 오브젝트(Components) 복제
    NewActor->DuplicateSubObjects();

    return NewActor;
}

// 서브 오브젝트(Components) 복제
void AActor::DuplicateSubObjects()
{
    // Components를 깊은 복사
    TSet<UActorComponent*> OriginalComponents = Components;
    Components.clear();

    for (UActorComponent* OriginalComp : OriginalComponents)
    {
        if (OriginalComp)
        {
            // TODO: UActorComponent::Duplicate() 구현 필요
            // UActorComponent* NewComp = Cast<UActorComponent>(OriginalComp->Duplicate());
            // if (NewComp)
            // {
            //     NewComp->SetOwner(this);
            //     AddComponent(NewComp);
            // }
        }
    }
}
