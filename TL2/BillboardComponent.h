#pragma once
#include "MeshComponent.h"
class UBillboardComponent : public UMeshComponent
{
public:
	DECLARE_CLASS(UBillboardComponent, UMeshComponent)
	UBillboardComponent();

protected:
	~UBillboardComponent() override;

public:
	TArray<FBillboardVertexInfo_GPU> CreateVertices(const FVector& StartPos);
	virtual void Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj) override;

	UTextQuad* GetStaticMesh() const { return Quad; }

private:
	UTextQuad* Quad = nullptr;
};

