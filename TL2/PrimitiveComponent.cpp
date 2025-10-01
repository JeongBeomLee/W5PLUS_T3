#include "pch.h"
#include "PrimitiveComponent.h"
#include "SceneLoader.h"
#include "SceneComponent.h"
#include "SceneRotationUtils.h"

void UPrimitiveComponent::SetMaterial(const FString& FilePath, EVertexLayoutType layoutType)
{
    Material = UResourceManager::GetInstance().Load<UMaterial>(FilePath, layoutType);
}

void UPrimitiveComponent::Serialize(bool bIsLoading, FPrimitiveData& InOut)
{
    if (bIsLoading)
    {
        // FPrimitiveData -> 컴포넌트 월드 트랜스폼
        SetRelativeLocation(InOut.Location);
        SetRelativeRotation(FQuat::MakeFromEuler(InOut.Rotation));
        SetRelativeScale(InOut.Scale);
    }
    else
    {
        // 컴포넌트 월드 트랜스폼 -> FPrimitiveData
        InOut.Location = GetRelativeLocation();
        InOut.Rotation = GetRelativeRotation().ToEulerDegree();
        InOut.Scale = GetRelativeScale();
    }
}

UObject* UPrimitiveComponent::Duplicate()
{
    // 부모 클래스의 Duplicate 호출 (Transform 복사)
    UPrimitiveComponent* NewComponent = static_cast<UPrimitiveComponent*>(Super_t::Duplicate());

    if (!NewComponent)
    {
        return nullptr;
    }

    // Material 포인터 복사 (리소스는 공유)
    NewComponent->Material = this->Material;

    return NewComponent;
}
void UPrimitiveComponent::RenderDetail()
{
    USceneComponent::RenderDetail();
}