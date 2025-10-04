#pragma once

#include "ResourceBase.h"
#include "Enums.h"
#include "StaticMesh.h"
#include <d3d11.h>

class UQuad : public UStaticMesh
{
public:
    DECLARE_CLASS(UQuad, UStaticMesh)

    UQuad() = default;
    virtual ~UQuad() override;
    virtual void Load(FMeshData* InData, ID3D11Device* InDevice, EVertexLayoutType InVertexType= EVertexLayoutType::PositionBillBoard) override;
    
protected:
    virtual void CreateVertexBuffer(FMeshData* InMeshData, ID3D11Device* InDevice, EVertexLayoutType InVertexType) override;
    virtual void CreateIndexBuffer(FMeshData* InMeshData, ID3D11Device* InDevice) override;
    virtual void ReleaseResources() override;
};