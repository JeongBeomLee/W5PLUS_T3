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

void UBillboardComponent::RenderDetail()
{
	UMeshComponent::RenderDetail();

	// jft
	if (ImGui::TreeNode("Sprite"))
	{
		auto& RM = UResourceManager::GetInstance();

		// 1) 'Icon'이름을 포함하는 텍스처 풀 목록 만들기
		const TArray<FString> AllTexNames = RM.GetAllFilePaths<UTexture>();
		static TArray<FString> IconTexNames;
		IconTexNames.clear();
		for (const FString& Name : AllTexNames)
		{
			if (Name.find("Icon") != std::string::npos)
			{
				IconTexNames.push_back(Name);
			}
		}

		// 2) ImGui용 char* 배열
		static TArray<const char*> Items;
		Items.clear();
		Items.reserve(IconTexNames.size());
		for (const FString& S : IconTexNames)
			Items.push_back(S.c_str());

		// 3) 현재 선택 인덱스 유지
		static int SelectedIdx = -1;

		// 현재 머티리얼의 텍스처 이름으로 동기화(처음 1회 혹은 버튼으로)
		if (SelectedIdx < 0)
		{
			const FString CurrentName = IconPath;
			for (int i = 0; i < static_cast<int>(IconTexNames.size()); ++i)
			{
				if (IconTexNames[i] == CurrentName)
				{
					SelectedIdx = i;
					break;
				}
			}
		}

		ImGui::SetNextItemWidth(280);
		ImGui::Combo("Icon Texture", &SelectedIdx,
			Items.data(), static_cast<int>(Items.size()));

		ImGui::SameLine();
		if (ImGui::Button("Apply Texture"))
		{
			if (SelectedIdx >= 0 && SelectedIdx < static_cast<int>(IconTexNames.size()))
			{
				IconPath = IconTexNames[SelectedIdx];
				if (UTexture* NewTex = RM.Load<UTexture>(IconPath))
				{
					if (!Material)
					{
						Material = NewObject<UMaterial>();
						RM.Add<UMaterial>("IconBillboard", Material);
					}
					Material->SetTexture(NewTex);
					UE_LOG("Applied Icon Texture: %s", IconPath.c_str());
				}
			}
		}
		// 읽기 전용 현재 텍스처 이름 표시
		// %s는 char pointer 필요
		IconPath = "필요";
		ImGui::Text("Current: %s", IconPath.c_str());

		ImGui::TreePop();
	}
}

void UBillboardComponent::Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj)
{
	// jft : change hard coded path
	Material->Load(IconPath, Renderer->GetRHIDevice()->GetDevice());
	AActor* Owner = GetOwner();
	UWorld* World = Owner->GetWorld();	
	ACameraActor* CameraActor = World->GetCameraActor();
	FVector CamRight = CameraActor->GetActorRight();
	FVector CamUp = CameraActor->GetActorUp();

	FVector CameraPosition = CameraActor->GetActorLocation();
	Renderer->UpdateBillboardConstantBuffers(GetWorldLocation() + FVector(0.f, 0.f, 1.f) * Owner->GetActorScale().Z, View, Proj, CamRight, CamUp);


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



