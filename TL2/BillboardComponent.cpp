#include "pch.h"
#include "BillboardComponent.h"
#include "ResourceManager.h"
#include "VertexData.h"
#include "CameraActor.h"

UBillboardComponent::UBillboardComponent()
{
	TArray<uint32> Indices = { 0, 1, 2, 1, 3, 2};

	auto& ResourceManager = UResourceManager::GetInstance();
	Quad = ResourceManager.Get<UTextQuad>("IconBillboard");
	if (auto* InMaterial = ResourceManager.Get<UMaterial>("IconBillboard"))
	{
		Material = InMaterial;
	}
	else
	{
		Material = NewObject<UMaterial>();
		ResourceManager.Add<UMaterial>("IconBillboard", Material);
	}
}

UBillboardComponent::~UBillboardComponent() {}

TArray<FBillboardVertexInfo_GPU> UBillboardComponent::CreateVertices(const FVector& StartPos)
{
	return TArray<FBillboardVertexInfo_GPU>();
}

void UBillboardComponent::Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj)
{
	Material->Load("Pawn_64x.dds", Renderer->GetRHIDevice()->GetDevice());
	ACameraActor* CameraActor = GetOwner()->GetWorld()->GetCameraActor();
	FVector CamRight = CameraActor->GetActorRight();
	FVector CamUp = CameraActor->GetActorUp();

	FVector CameraPosition = CameraActor->GetActorLocation();
	Renderer->UpdateBillboardConstantBuffers(Owner->GetActorLocation() + FVector(0.f, 0.f, 1.f) * Owner->GetActorScale().Z, View, Proj, CamRight, CamUp);


	Renderer->PrepareShader(Material->GetShader());
	TArray<FBillboardVertexInfo_GPU> vertices; 
	vertices.SetNum(4);

	UTexture* Texture = Material->GetTexture();
	float AspectRatio = 1.0f; // 기본 가로/세로 비율

	if (Texture && Texture->GetHeight() > 0)
	{
		AspectRatio = static_cast<float>(Texture->GetWidth()) / static_cast<float>(Texture->GetHeight());
	}

	float HalfWidth = AspectRatio * 0.5f;
	float HalfHeight = 0.5f;

	// 정점 0: 왼쪽 위 (Top-Left)
	vertices[0].Position[0] = -HalfWidth;
	vertices[0].Position[1] = HalfHeight;
	vertices[0].Position[2] = 0.f;
	vertices[0].CharSize[0] = 1.f; // TextRenderComponent와 동일하게 1.0으로 설정
	vertices[0].CharSize[1] = 1.f;
	vertices[0].UVRect[0] = 0.f; // UV: (0, 0)
	vertices[0].UVRect[1] = 0.f;

	// 정점 1: 오른쪽 위 (Top-Right)
	vertices[1].Position[0] = HalfWidth;
	vertices[1].Position[1] = HalfHeight;
	vertices[1].Position[2] = 0.f;
	vertices[1].CharSize[0] = 1.f;
	vertices[1].CharSize[1] = 1.f;
	vertices[1].UVRect[0] = 1.f; // UV: (1, 0)
	vertices[1].UVRect[1] = 0.f;

	// 정점 2: 왼쪽 아래 (Bottom-Left)
	vertices[2].Position[0] = -HalfWidth;
	vertices[2].Position[1] = -HalfHeight;
	vertices[2].Position[2] = 0.f;
	vertices[2].CharSize[0] = 1.f;
	vertices[2].CharSize[1] = 1.f;
	vertices[2].UVRect[0] = 0.f; // UV: (0, 1)
	vertices[2].UVRect[1] = 1.f;

	// 정점 3: 오른쪽 아래 (Bottom-Right)
	vertices[3].Position[0] = HalfWidth;
	vertices[3].Position[1] = -HalfHeight;
	vertices[3].Position[2] = 0.f;
	vertices[3].CharSize[0] = 1.f;
	vertices[3].CharSize[1] = 1.f;
	vertices[3].UVRect[0] = 1.f; // UV: (1, 1)
	vertices[3].UVRect[1] = 1.f;

	UResourceManager::GetInstance().UpdateDynamicVertexBuffer("IconBillboard", vertices);
	Renderer->OMSetBlendState(true);
	Renderer->RSSetState(EViewModeIndex::VMI_Unlit);
	Renderer->DrawIndexedPrimitiveComponent(this, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Renderer->OMSetBlendState(false);
}


