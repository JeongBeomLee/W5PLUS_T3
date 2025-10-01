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
	Super_t::Tick(DeltaTime);
	if (bIsPicked && CollisionComponent)
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
    NewActor->bIsPicked = false;
    NewActor->bCanEverTick = this->bCanEverTick;
    NewActor->bHiddenInGame = this->bHiddenInGame;
    NewActor->bTickInEditor = this->bTickInEditor;

    // 원본(this)의 컴포넌트를 NewActor에 임시 설정
    TSet<UActorComponent*> OriginalComponents = this->Components;
    USceneComponent* OriginalRootComponent = this->RootComponent;

    NewActor->Components = OriginalComponents;
    NewActor->RootComponent = OriginalRootComponent;

    // 서브 오브젝트(Components) 복제
    NewActor->DuplicateSubObjects();

    return NewActor;
}

// AStaticMeshActor 서브 오브젝트 복제
void AStaticMeshActor::DuplicateSubObjects()
{
    // 부모 클래스의 DuplicateSubObjects 호출 (컴포넌트 복제 및 계층 구조 복원)
    Super_t::DuplicateSubObjects();

    // AStaticMeshActor 전용 멤버 포인터 재설정
    StaticMeshComponent = nullptr;
    CollisionComponent = nullptr;

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