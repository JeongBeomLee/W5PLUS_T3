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
    // 같은 타입의 새 액터 생성
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

    // Transform 복사
    if (this->RootComponent)
    {
        NewActor->SetActorTransform(this->GetActorTransform());
    }

    // 서브 오브젝트(Components) 복제
    NewActor->DuplicateSubObjects();

    return NewActor;
}