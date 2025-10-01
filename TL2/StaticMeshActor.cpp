#include "pch.h"
#include "AABoundingBoxComponent.h"
#include "StaticMeshActor.h"
#include "ObjectFactory.h"

AStaticMeshActor::AStaticMeshActor()
{
    Name = "Static Mesh Actor";
    StaticMeshComponent = NewObject<UStaticMeshComponent>();
    AddComponent(StaticMeshComponent);
    StaticMeshComponent->SetupAttachment(RootComponent);

    CollisionComponent = CreateDefaultSubobject<UAABoundingBoxComponent>(FName("CollisionBox"));
    AddComponent(CollisionComponent);
	CollisionComponent->SetupAttachment(RootComponent);
}

void AStaticMeshActor::Tick(float DeltaTime)
{
    if(bIsPicked&& CollisionComponent)
    CollisionComponent->SetFromVertices(StaticMeshComponent->GetStaticMesh()->GetStaticMeshAsset()->Vertices);
}

AStaticMeshActor::~AStaticMeshActor()
{
    if (StaticMeshComponent)
    {
        ObjectFactory::DeleteObject(StaticMeshComponent);
    }
    StaticMeshComponent = nullptr;
}

void AStaticMeshActor::SetStaticMeshComponent(UStaticMeshComponent* InStaticMeshComponent)
{
    StaticMeshComponent = InStaticMeshComponent;
}

void AStaticMeshActor::SetCollisionComponent(EPrimitiveType InType)
{
    if (!CollisionComponent) {
        return;
    }
    CollisionComponent->SetFromVertices(StaticMeshComponent->GetStaticMesh()->GetStaticMeshAsset()->Vertices);
    CollisionComponent->SetPrimitiveType(InType);
}

// AStaticMeshActor 복제
UObject* AStaticMeshActor::Duplicate()
{
    // 새 액터 생성 (올바른 타입으로)
    AStaticMeshActor* NewActor = NewObject<AStaticMeshActor>();
    if (!NewActor)
    {
        return nullptr;
    }

    // 기본 프로퍼티 복사
    NewActor->Name = this->Name;
    NewActor->bIsPicked = false;  // 복제본은 선택 해제
    NewActor->bCanEverTick = this->bCanEverTick;
    NewActor->bHiddenInGame = this->bHiddenInGame;
    NewActor->bTickInEditor = this->bTickInEditor;

    // 생성자에서 만든 NewActor의 기본 컴포넌트 제거
    if (NewActor->StaticMeshComponent)
    {
        NewActor->Components.erase(NewActor->StaticMeshComponent);
        ObjectFactory::DeleteObject(NewActor->StaticMeshComponent);
        NewActor->StaticMeshComponent = nullptr;
    }
    if (NewActor->CollisionComponent)
    {
        NewActor->Components.erase(NewActor->CollisionComponent);
        ObjectFactory::DeleteObject(NewActor->CollisionComponent);
        NewActor->CollisionComponent = nullptr;
    }

    // 원본(this)의 컴포넌트를 NewActor에 임시 설정하여 DuplicateSubObjects가 복제할 수 있도록 함
    TSet<UActorComponent*> OriginalComponents = this->Components;
    USceneComponent* OriginalRootComponent = this->RootComponent;

    NewActor->Components = OriginalComponents;
    NewActor->RootComponent = OriginalRootComponent;

    // 서브 오브젝트(Components) 복제
    NewActor->DuplicateSubObjects();

    // Transform 복사
    if (this->RootComponent && NewActor->RootComponent)
    {
        NewActor->SetActorTransform(this->RootComponent->GetRelativeTransform());
    }

    return NewActor;
}

// AStaticMeshActor 서브 오브젝트 복제
void AStaticMeshActor::DuplicateSubObjects()
{
    // 이 시점에서 Components는 원본 액터의 컴포넌트를 가리키고 있음 (Duplicate()에서 설정됨)

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
                AddComponent(NewComp);
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

    // AStaticMeshActor 전용 멤버 포인터 설정
    for (UActorComponent* Comp : Components)
    {
        if (!StaticMeshComponent)
        {
            StaticMeshComponent = Cast<UStaticMeshComponent>(Comp);
        }
        if (!CollisionComponent)
        {
            CollisionComponent = Cast<UAABoundingBoxComponent>(Comp);
        }
    }
}