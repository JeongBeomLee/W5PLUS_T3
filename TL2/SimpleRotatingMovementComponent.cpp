#include "pch.h"
#include "SimpleRotatingMovementComponent.h"
#include "Actor.h"

USimpleRotatingMovementComponent::USimpleRotatingMovementComponent()
{
    bCanEverTick = true;  // Tick 활성화
}

USimpleRotatingMovementComponent::~USimpleRotatingMovementComponent()
{
}

void USimpleRotatingMovementComponent::BeginPlay()
{
    UActorComponent::BeginPlay();
}

void USimpleRotatingMovementComponent::TickComponent(float DeltaTime)
{
    UActorComponent::TickComponent(DeltaTime);

    if (!Owner)
        return;

    // Z축 기준으로 회전
    float DeltaRotation = RotationRate * DeltaTime;

    // 현재 회전값 가져오기
    FQuat CurrentRotation = Owner->GetActorRotation();

    // 델타 회전 생성 (Z축)
    FQuat DeltaQuat = FQuat::MakeFromEuler(FVector(0.0f, 0.0f, DeltaRotation));

    // 현재 회전에 델타 회전 추가
    FQuat NewRotation = CurrentRotation * DeltaQuat;
    Owner->SetActorRotation(NewRotation);
}

UObject* USimpleRotatingMovementComponent::Duplicate()
{
    USimpleRotatingMovementComponent* NewComponent =
        static_cast<USimpleRotatingMovementComponent*>(UActorComponent::Duplicate());

    if (NewComponent)
    {
        // 회전 속도 복사
        NewComponent->RotationRate = this->RotationRate;
    }

    return NewComponent;
}
