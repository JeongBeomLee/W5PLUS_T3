#pragma once
#include "ActorComponent.h"
#include "Vector.h"

class USimpleRotatingMovementComponent : public UActorComponent
{
public:
    DECLARE_CLASS(USimpleRotatingMovementComponent, UActorComponent)

    USimpleRotatingMovementComponent();
    virtual ~USimpleRotatingMovementComponent() override;

    // Lifecycle
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime) override;

    // 복제 시스템
    virtual UObject* Duplicate() override;

    // 회전 속도 설정 (도/초)
    void SetRotationRate(float DegreesPerSecond) { RotationRate = DegreesPerSecond; }
    float GetRotationRate() const { return RotationRate; }

private:
    float RotationRate = 90.0f;  // 기본 회전 속도: 90도/초
};
