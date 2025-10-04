#include "pch.h"
#include "MeshComponent.h"
#include "StaticMesh.h"
#include "ObjManager.h"

UMeshComponent::UMeshComponent()

{
}

UMeshComponent::~UMeshComponent()
{
    Material = nullptr;
}
void UMeshComponent::RenderDetail()
{
    UPrimitiveComponent::RenderDetail();
}