#pragma once
#include "StaticMeshComponent.h"

// Match With GPU's BillboardInfo
struct BillboardBufferType
{
	FMatrix ViewMatrix;
	FMatrix ProjectionMatrix;
	
	FVector WorldPos;
	FMatrix InverseViewMat;
	float TextureHalfWidth;
	float TextureHalfHeight;
	float Scale;
};

class UBillboardComponent : public UStaticMeshComponent
{
public:
	DECLARE_CLASS(UBillboardComponent, UStaticMeshComponent)
	UBillboardComponent();

protected:
	~UBillboardComponent() override;

	virtual void RenderDetail() override;
public:
	virtual void Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj) override;
	static void InitializeMesh();

private:
	bool bIsTextureDirty = true;
	FString IconPath = "Editor/Icon/Pawn_64x.dds";

	bool bIsScaleDirty = true;
	
	BillboardBufferType BillboardBuffer;
};

