#include "pch.h"
#include "SceneComponent.h"
#include <algorithm>
#include "ObjectFactory.h"
#include "ImGui/imgui.h"

USceneComponent::USceneComponent()
    : AttachParent(nullptr)
{

}

USceneComponent::~USceneComponent()
{
    // 자식 메모리 해제
    // 복사본을 만들어 부모 리스트 무효화 문제를 피함
    TArray<USceneComponent*> ChildrenCopy = AttachChildren;
    AttachChildren.clear();
    for (USceneComponent* Child : ChildrenCopy)
    {
        if (Child)
        {
            ObjectFactory::DeleteObject(Child);
        }
    }

    // 부모에서 자신 제거
    if (AttachParent)
    {
        TArray<USceneComponent*>& ParentChildren = AttachParent->AttachChildren;
        ParentChildren.Remove(this);
        AttachParent = nullptr;
    }
}

// ──────────────────────────────
// Relative API
// ──────────────────────────────
void USceneComponent::SetRelativeLocation(const FVector& NewLocation) {
    RelativeTransform.Translation = NewLocation;
    TransformDirty();
}
FVector USceneComponent::GetRelativeLocation() const { return  RelativeTransform.Translation; }

void USceneComponent::SetRelativeRotation(const FQuat& NewRotation)
{
    RelativeTransform.Rotation = NewRotation;
    TransformDirty();
}
FQuat USceneComponent::GetRelativeRotation() const { return  RelativeTransform.Rotation; }

void USceneComponent::SetRelativeScale(const FVector& NewScale)
{
    RelativeTransform.Scale3D = NewScale;
    TransformDirty();
}
FVector USceneComponent::GetRelativeScale() const { return  RelativeTransform.Scale3D; }


void USceneComponent::SetWorldLocation(const FVector& WorldLocation)
{
    TransformDirty();
    if (AttachParent != nullptr)
    {
        FMatrix InverseParentWorld = AttachParent->GetWorldMatrix().InverseAffine();
        RelativeTransform.Translation = WorldLocation * InverseParentWorld;
    }
    else
    {
        RelativeTransform.Translation = WorldLocation;
    }
}
FVector USceneComponent::GetWorldLocation()
{
    if (AttachParent != nullptr)
    {
        const FMatrix& ParentWorldMatrix = AttachParent->GetWorldMatrix();
        FVector temp = RelativeTransform.Translation * ParentWorldMatrix;
        return RelativeTransform.Translation * ParentWorldMatrix;
    }
    return RelativeTransform.Translation;
}

void USceneComponent::SetRelativeTransform(const FTransform& InRelativeTransform)
{
    RelativeTransform = InRelativeTransform;
    TransformDirty();
}
FTransform USceneComponent::GetRelativeTransform()const
{
    return RelativeTransform;
}
void USceneComponent::TransformDirty()
{
    bTransformDirty = true;
    for (USceneComponent* Child : AttachChildren)
    {
        Child->TransformDirty();
    }
}
FVector USceneComponent::GetForward()
{
    if (AttachParent != nullptr)
    {
        const FMatrix& ParentWorldMatrix = AttachParent->GetWorldMatrix();
        FMatrix CurRotMatrix = RelativeTransform.Rotation.ToMatrix() * ParentWorldMatrix;
        return CurRotMatrix.GetForward();
    }
    return RelativeTransform.Rotation.GetForward();
}
FVector USceneComponent::GetRight()
{
    if (AttachParent != nullptr)
    {
        const FMatrix& ParentWorldMatrix = AttachParent->GetWorldMatrix();
        FMatrix CurRotMatrix = RelativeTransform.Rotation.ToMatrix() * ParentWorldMatrix;
        return CurRotMatrix.GetRight();
    }
    return RelativeTransform.Rotation.GetRight();
}
FVector USceneComponent::GetUp()
{
    if (AttachParent != nullptr)
    {
        const FMatrix& ParentWorldMatrix = AttachParent->GetWorldMatrix();
        FMatrix CurRotMatrix = RelativeTransform.Rotation.ToMatrix() * ParentWorldMatrix;
        return CurRotMatrix.GetUp();
    }
    return RelativeTransform.Rotation.GetUp();
}
const FMatrix& USceneComponent::GetWorldMatrix()
{
    if (bTransformDirty)
    {
        bTransformDirty = false;
        if (AttachParent != nullptr)
        {
            WorldMatrix = RelativeTransform.GetWorldMatrix() * AttachParent->GetWorldMatrix();
        }
        else
        {
            WorldMatrix = RelativeTransform.GetWorldMatrix();
        }
    }
    return WorldMatrix;
}

// ──────────────────────────────
// Attach / Detach
// ──────────────────────────────
void USceneComponent::SetupAttachment(USceneComponent* InParent)
{
    if (AttachParent == InParent) return;

    // 기존 부모에서 제거
    if (AttachParent)
    {
        auto& Siblings = AttachParent->AttachChildren;
        Siblings.erase(std::remove(Siblings.begin(), Siblings.end(), this), Siblings.end());
    }

    // 새 부모 설정
    AttachParent = InParent;
    if (AttachParent)
        AttachParent->AttachChildren.push_back(this);

    // 규칙 적용

    TransformDirty();
}

void USceneComponent::DetachFromParent(bool bKeepWorld)
{
    if (AttachParent)
    {
        auto& Siblings = AttachParent->AttachChildren;
        Siblings.erase(std::remove(Siblings.begin(), Siblings.end(), this), Siblings.end());
        AttachParent = nullptr;
    }

    TransformDirty();

}


// ──────────────────────────────
// 복제 시스템
// ──────────────────────────────
UObject* USceneComponent::Duplicate()
{
    // 부모 클래스의 Duplicate 호출 (기본 속성 복사)
    //USceneComponent* NewComponent = static_cast<USceneComponent*>(UActorComponent::Duplicate());
    USceneComponent* NewComponent = static_cast<USceneComponent*>(Super_t::Duplicate());

    if (!NewComponent)
    {
        return nullptr;
    }

    // Transform 정보 복사
    NewComponent->SetRelativeLocation(this->GetRelativeLocation());
    NewComponent->SetRelativeRotation(this->GetRelativeRotation());
    NewComponent->SetRelativeScale(this->GetRelativeScale());

    // AttachParent와 AttachChildren은 복제 후 외부에서 설정
    // (계층 구조 재구성은 Actor 레벨에서 처리)

    return NewComponent;
}

void USceneComponent::RenderDetail()
{
    UActorComponent::RenderDetail();

    if (ImGui::TreeNode("Transform"))
    {
        // Location 편집
        if (ImGui::DragFloat3("Location", &RelativeTransform.Translation.X, 0.1f))
        {
            SetRelativeLocation(RelativeTransform.Translation);
        }

        // Rotation 편집 (Euler angles)
        FVector Euler = RelativeTransform.Rotation.ToEulerDegree();
        if (ImGui::DragFloat3("Rotation", &Euler.X, 0.5f))
        {
            SetRelativeRotation(FQuat::MakeFromEuler(Euler));
        }

        // Scale 편집
        ImGui::Checkbox("Uniform Scale", &bUniformScale);

        if (bUniformScale)
        {
            float UniformScale = RelativeTransform.Scale3D.X;
            if (ImGui::DragFloat("Scale", &UniformScale, 0.01f, 0.01f, 10.0f))
            {
                SetRelativeScale(FVector(UniformScale, UniformScale, UniformScale));
            }
        }
        else
        {
            if (ImGui::DragFloat3("Scale", &RelativeTransform.Scale3D.X, 0.01f, 0.01f, 10.0f))
            {
                SetRelativeScale(RelativeTransform.Scale3D);
            }
        }
        ImGui::TreePop();
    }
}