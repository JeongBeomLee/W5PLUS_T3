#pragma once
#include "ResourceBase.h"
#include "Enums.h"
#include <d3d11.h>
#include "BoundingVolumeHierarchy.h"

class UStaticMesh : public UResourceBase
{
public:
    DECLARE_CLASS(UStaticMesh, UResourceBase)

    UStaticMesh() = default;
    virtual ~UStaticMesh() override;

    virtual void Load(const FString& InFilePath, ID3D11Device* InDevice, EVertexLayoutType InVertexType = EVertexLayoutType::PositionColorTextureNormal);
    virtual void Load(FMeshData* InData, ID3D11Device* InDevice, EVertexLayoutType InVertexType = EVertexLayoutType::PositionColorTextureNormal);

    ID3D11Buffer* GetVertexBuffer() const { return VertexBuffer; }
    ID3D11Buffer* GetIndexBuffer() const { return IndexBuffer; }
    uint32 GetVertexCount() { return VertexCount; }
    uint32 GetIndexCount() { return IndexCount; }
    EVertexLayoutType GetVertexType() const { return VertexType; }
    void SetIndexCount(uint32 Cnt) { IndexCount = Cnt; }

	const FString& GetAssetPathFileName() const { return StaticMeshAsset ? StaticMeshAsset->PathFileName : FilePath; }
    void SetStaticMeshAsset(FStaticMesh* InStaticMesh) { StaticMeshAsset = InStaticMesh; }
	FStaticMesh* GetStaticMeshAsset() const { return StaticMeshAsset; }

    const TArray<FGroupInfo>& GetMeshGroupInfo() const { return StaticMeshAsset->GroupInfos; }
    bool HasMaterial() const { return StaticMeshAsset->bHasMaterial; }

    uint64 GetMeshGroupCount() const { return StaticMeshAsset->GroupInfos.size(); }

    void BuildMeshBVH();
    FNarrowPhaseBVHNode* GetMeshBVH() const { return MeshBVH; }

protected:
    virtual void CreateVertexBuffer(FMeshData* InMeshData, ID3D11Device* InDevice, EVertexLayoutType InVertexType);
	virtual void CreateVertexBuffer(FStaticMesh* InStaticMesh, ID3D11Device* InDevice, EVertexLayoutType InVertexType);
    virtual void CreateIndexBuffer(FMeshData* InMeshData, ID3D11Device* InDevice);
	virtual void CreateIndexBuffer(FStaticMesh* InStaticMesh, ID3D11Device* InDevice);
    virtual void ReleaseResources();

    // GPU 리소스
    ID3D11Buffer* VertexBuffer = nullptr;
    ID3D11Buffer* IndexBuffer = nullptr;
    uint32 VertexCount = 0;     // 정점 개수
    uint32 IndexCount = 0;     // 버텍스 점의 개수
    EVertexLayoutType VertexType = EVertexLayoutType::PositionColorTextureNormal;  // 버텍스 타입

	// CPU 리소스
    FStaticMesh* StaticMeshAsset = nullptr;

    // mesh 단위 BVH 루트 노드
    FNarrowPhaseBVHNode* MeshBVH = nullptr;
};