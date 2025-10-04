#pragma once

#include "pch.h"
#include "TextQuad.h"
#include "MeshLoader.h"
#include "ResourceManager.h"
#include "ObjManager.h"

UQuad::~UQuad()
{
    ReleaseResources();
}

void UQuad::Load(FMeshData* InData, ID3D11Device* InDevice, EVertexLayoutType InVertexType)
{
    if (VertexBuffer)
    {
        VertexBuffer->Release();
    }
    if (IndexBuffer)
    {
        IndexBuffer->Release();
    }
    
    CreateVertexBuffer(InData, InDevice, InVertexType);
    CreateIndexBuffer(InData, InDevice);

    VertexCount = static_cast<uint32>(InData->Position.size());
    IndexCount = static_cast<uint32>(InData->Indices.size());
}

void UQuad::CreateVertexBuffer(FMeshData* InMeshData, ID3D11Device* InDevice, EVertexLayoutType InVertexType)
{
    // InVertexType is Useless Overall =>  FBillboardVertexInfo_GPU Input Type is HardCoded
    // RHI is Useless Just for now => Directly uses D3D11RHI
    HRESULT hr = D3D11RHI::CreateVertexBuffer<FBillboardVertexInfo_GPU>(InDevice, *InMeshData, &VertexBuffer);
    assert(SUCCEEDED(hr));
}

void UQuad::CreateIndexBuffer(FMeshData* InMeshData, ID3D11Device* InDevice)
{
    HRESULT hr = D3D11RHI::CreateIndexBuffer(InDevice, InMeshData, &IndexBuffer);
    assert(SUCCEEDED(hr));
}

void UQuad::ReleaseResources()
{
    if (VertexBuffer)
    {
        VertexBuffer->Release();
    }
    if (IndexBuffer)
    {
        IndexBuffer->Release();
    }
}