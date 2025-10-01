#pragma once
#include "Actor.h"
#include "StaticMeshComponent.h"
#include "SimpleRotatingMovementComponent.h"
#include "Enums.h"

class AStaticMeshActor : public AActor
{
public:
    DECLARE_CLASS(AStaticMeshActor, AActor)

    AStaticMeshActor();
    virtual void Tick(float DeltaTime) override;

    // 복제 시스템
    virtual UObject* Duplicate() override;
    virtual void DuplicateSubObjects() override;

protected:
    ~AStaticMeshActor() override;

public:
    UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }
    void SetStaticMeshComponent(UStaticMeshComponent* InStaticMeshComponent);
	void SetCollisionComponent(EPrimitiveType InType = EPrimitiveType::Default);

    USimpleRotatingMovementComponent* GetRotatingMovementComponent() const { return RotatingMovementComponent; }

protected:
    UStaticMeshComponent* StaticMeshComponent;
    USimpleRotatingMovementComponent* RotatingMovementComponent;
};

