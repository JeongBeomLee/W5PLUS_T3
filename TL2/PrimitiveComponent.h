#pragma once
#include "SceneComponent.h"
#include "Material.h"

// 전방 선언
struct FPrimitiveData;

class URenderer;

class UPrimitiveComponent :public USceneComponent
{
public:
    DECLARE_CLASS(UPrimitiveComponent, USceneComponent)

    UPrimitiveComponent() = default;
    virtual ~UPrimitiveComponent() = default;

    virtual void SetMaterial(const FString& FilePath, EVertexLayoutType layoutType);
    virtual UMaterial* GetMaterial() { return Material; }

    // 트랜스폼 직렬화/역직렬화 (월드 트랜스폼 기준)
    virtual void Serialize(bool bIsLoading, FPrimitiveData& InOut);

    virtual void Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj) {}

    // 복제 시스템
    virtual UObject* Duplicate() override;
protected:
    virtual void RenderDetail() override;

protected:
    UMaterial* Material = nullptr;
};