#include "pch.h"
#include "StaticMesh.h"
#include "ObjManager.h"
#include "Triangle.h"

UStaticMesh::~UStaticMesh()
{
    // 기존 BVH가 있다면 해제
    if (MeshBVH)
    {
        FBVHBuilder Builder;
        Builder.Cleanup<FNarrowPhaseBVHNode>(MeshBVH);
        MeshBVH = nullptr;
    }
    ReleaseResources();
}

void UStaticMesh::Load(const FString& InFilePath, ID3D11Device* InDevice, EVertexLayoutType InVertexType)
{
    assert(InDevice);

    VertexType = InVertexType;

    StaticMeshAsset = FObjManager::LoadObjStaticMeshAsset(InFilePath);
    CreateVertexBuffer(StaticMeshAsset, InDevice, InVertexType);
    CreateIndexBuffer(StaticMeshAsset, InDevice);
    VertexCount = static_cast<uint32>(StaticMeshAsset->Vertices.size());
    IndexCount = static_cast<uint32>(StaticMeshAsset->Indices.size());
}

void UStaticMesh::Load(FMeshData* InData, ID3D11Device* InDevice, EVertexLayoutType InVertexType)
{
    VertexType = InVertexType;

    if (VertexBuffer)
    {
        VertexBuffer->Release();
        VertexBuffer = nullptr;
    }
    if (IndexBuffer)
    {
        IndexBuffer->Release();
        IndexBuffer = nullptr;
    }

    CreateVertexBuffer(InData, InDevice, InVertexType);
    CreateIndexBuffer(InData, InDevice);

    VertexCount = static_cast<uint32>(InData->Vertices.size());
    IndexCount = static_cast<uint32>(InData->Indices.size());
}

void UStaticMesh::BuildMeshBVH()
{
    if (MeshBVH)
    {
        FBVHBuilder Builder;
        Builder.Cleanup<FNarrowPhaseBVHNode>(MeshBVH);
        MeshBVH = nullptr;
    }
    if (!StaticMeshAsset || StaticMeshAsset->Indices.empty() || StaticMeshAsset->Vertices.empty())
    {
        return;
    }

    TArray<FNarrowPhaseBVHPrimitive> Primitives;
    const int32 TriangleCount = StaticMeshAsset->Indices.size() / 3;
    Primitives.Reserve(TriangleCount);

    for (int32 i = 0;i < TriangleCount;++i)
    {
        // 삼각형을 구성하는 세 인덱스
        const uint32 Index0 = StaticMeshAsset->Indices[i * 3 + 0];
        const uint32 Index1 = StaticMeshAsset->Indices[i * 3 + 1];
        const uint32 Index2 = StaticMeshAsset->Indices[i * 3 + 2];

        // 인덱스에 해당하는 실제 정점 위치
        const FVector& V0 = StaticMeshAsset->Vertices[Index0].pos;
        const FVector& V1 = StaticMeshAsset->Vertices[Index1].pos;
        const FVector& V2 = StaticMeshAsset->Vertices[Index2].pos;

        // FTriangle을 이용해 이 삼각형의 경계 상자(AABB) 계산
        FBound TriangleBounds = FTriangle(V0, V1, V2).GetBounds();

        // BVH 프리미티브 생성 및 리스트에 추가
        FNarrowPhaseBVHPrimitive Primitive;
        Primitive.TriangleIndex = i;
        Primitive.Bounds = TriangleBounds;
        Primitives.Add(Primitive);
    }
    if (!Primitives.IsEmpty())
    {
        FBVHBuilder Builder;
        MeshBVH = Builder.Build<FNarrowPhaseBVHNode, FNarrowPhaseBVHPrimitive>(Primitives);

        char buf[256];
        sprintf_s(buf, "[BVH Build] Mesh '%s' BVH built for %d triangles.\n",
            GetAssetPathFileName().c_str(), TriangleCount);
        UE_LOG(buf);
    }
}

void UStaticMesh::CreateVertexBuffer(FMeshData* InMeshData, ID3D11Device* InDevice, EVertexLayoutType InVertexType)
{
    HRESULT hr;
    hr = D3D11RHI::CreateVertexBuffer<FVertexDynamic>(InDevice, *InMeshData, &VertexBuffer);
    assert(SUCCEEDED(hr));
}

void UStaticMesh::CreateVertexBuffer(FStaticMesh* InStaticMesh, ID3D11Device* InDevice, EVertexLayoutType InVertexType)
{
    HRESULT hr;
    hr = D3D11RHI::CreateVertexBuffer<FVertexDynamic>(InDevice, InStaticMesh->Vertices, &VertexBuffer);
    assert(SUCCEEDED(hr));
}

void UStaticMesh::CreateIndexBuffer(FMeshData* InMeshData, ID3D11Device* InDevice)
{
    HRESULT hr = D3D11RHI::CreateIndexBuffer(InDevice, InMeshData, &IndexBuffer);
    assert(SUCCEEDED(hr));
}

void UStaticMesh::CreateIndexBuffer(FStaticMesh* InStaticMesh, ID3D11Device* InDevice)
{
    HRESULT hr = D3D11RHI::CreateIndexBuffer(InDevice, InStaticMesh, &IndexBuffer);
    assert(SUCCEEDED(hr));
}

void UStaticMesh::ReleaseResources()
{
    if (VertexBuffer)
    {
        VertexBuffer->Release();
        VertexBuffer = nullptr;
    }
    if (IndexBuffer)
    {
        IndexBuffer->Release();
        IndexBuffer = nullptr;
    }
}

