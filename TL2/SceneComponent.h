#pragma once
#include "Vector.h"
#include "ActorComponent.h"



// 부착 시 로컬을 유지할지, 월드를 유지할지
enum class EAttachmentRule
{
    KeepRelative,
    KeepWorld
};

class URenderer;
class USceneComponent : public UActorComponent
{
public:
    DECLARE_CLASS(USceneComponent, UActorComponent)
    USceneComponent();

protected:
    ~USceneComponent() override;


public:
    // ──────────────────────────────
    // Relative Transform API
    // ──────────────────────────────
    void SetRelativeLocation(const FVector& NewLocation);
    FVector GetRelativeLocation() const;

    void SetRelativeRotation(const FQuat& NewRotation);
    FQuat GetRelativeRotation() const;

    void SetRelativeScale(const FVector& NewScale);
    FVector GetRelativeScale() const;

    void SetWorldLocation(const FVector& WorldLocation);
    FVector GetWorldLocation();
    void SetRelativeTransform(const FTransform& InRelativeTransform);
    FTransform GetRelativeTransform()const;

    FVector GetForward();
    FVector GetRight();
    FVector GetUp();

    const FMatrix& GetWorldMatrix(); // ToMatrixWithScale

    // ──────────────────────────────
    // Attach/Detach
    // ──────────────────────────────
    void SetupAttachment(USceneComponent* InParent);
    void DetachFromParent(bool bKeepWorld = true);

    // ──────────────────────────────
    // Hierarchy Access
    // ──────────────────────────────
    USceneComponent* GetAttachParent() const { return AttachParent; }
    const TArray<USceneComponent*>& GetAttachChildren() const { return AttachChildren; }
    UWorld* GetWorld() { return AttachParent->GetWorld(); }

    // ──────────────────────────────
    // 복제 시스템
    // ──────────────────────────────
    virtual UObject* Duplicate() override;
protected:
    virtual void RenderDetail() override;

protected:
    FTransform RelativeTransform;

    
    // Hierarchy
    USceneComponent* AttachParent = nullptr;
    TArray<USceneComponent*> AttachChildren;

    // 로컬(부모 기준) 트랜스폼
    FMatrix WorldMatrix;
private:
    void TransformDirty();
private:
    bool bUniformScale = true;
    bool bTransformDirty = true;
};
