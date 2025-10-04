#include "pch.h"
#include "BillboardComponent.h"
#include "ResourceManager.h"
#include "CameraActor.h"

UBillboardComponent::UBillboardComponent()
{
	// Register Mesh Data Just Once
	InitializeMesh();
	SetStaticMesh("IconBillboard");
	
	// Set Material's Shader, CPU InputLayout : FBillboardVertexInfo
	SetMaterial("Billboard.hlsl", EVertexLayoutType::PositionBillBoard);	
}

void UBillboardComponent::InitializeMesh()
{
	// Set MeshData => Same with FBillboardVertexInfo
	// Set Indices
	TArray<uint32> Indices = { 0, 1, 2, 1, 3, 2};

	// set Vertices
	TArray<FVector> Position = {/* Multiply Texture's HalfWidth, HalfHeight in Shader*/
		FVector(-1.f, 1.f, 0.f),  /* 정점 0: 왼쪽 위 (Top-Left) */
		FVector(1.f, 1.f, 0.f),   /*정점 1: 오른쪽 위 (Top-Right)*/
		FVector(-1.f, -1.f, 0.f), /* 정점 2: 왼쪽 아래 (Bottom-Left)*/
		FVector(1.f, -1.f, 0.f),  /* 정점 3: 오른쪽 아래 (Bottom-Right)*/
	};
	
	TArray<FVector2D> Scale =
	{
		FVector2D(1.f, 1.f),
		FVector2D(1.f, 1.f),
		FVector2D(1.f, 1.f),
		FVector2D(1.f, 1.f),
	};

	TArray<FVector4> UVRect =
	{
		FVector4(0.f, 0.f, 1.f, 1.f),
		FVector4(1.f, 0.f, 1.f, 1.f),
		FVector4(0.f, 1.f, 1.f, 1.f),
		FVector4(1.f, 1.f, 1.f, 1.f),
	};
	
	FMeshData* BillboardData = new FMeshData;
	BillboardData->Indices = Indices;
	BillboardData->Position = Position;
	BillboardData->Color = UVRect; //also can be UVRect
	BillboardData->UV = Scale;     //also can be Billboard Scale

	UResourceManager::GetInstance().Add<UQuad>("IconBillboard", BillboardData);
}

UBillboardComponent::~UBillboardComponent() {}

void UBillboardComponent::RenderDetail()
{
	UMeshComponent::RenderDetail();

	// todo : Remove ImGUI from RenderDetail would be nice..
	if (ImGui::TreeNode("Sprite"))
	{
		auto& ResourceManger = UResourceManager::GetInstance();

		// 1) 'Icon'이름을 포함하는 텍스처 풀 목록 만들기
		const TArray<FString> AllTexNames = ResourceManger.GetAllFilePaths<UTexture>();
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
				if (UTexture* NewTex = ResourceManger.Load<UTexture>(IconPath))
				{
					if (!Material)
					{
						Material = NewObject<UMaterial>();
						ResourceManger.Add<UMaterial>("IconBillboard", Material);
					}
					Material->SetTexture(NewTex);
					UE_LOG("Applied Icon Texture: %s", IconPath.c_str());
				}
			}
		}
		
		ImGui::Text("Current: %s", IconPath.c_str());
		ImGui::TreePop();
	}
}

void UBillboardComponent::Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj)
{
	// Set Texture
	if (bIsTextureDirty)
	{
		bIsTextureDirty = false;
		
		Material->Load(IconPath, Renderer->GetRHIDevice()->GetDevice());

		UTexture* Texture = Material->GetTexture();
		if (Texture && Texture->GetHeight() > 0)
		{
			float AspectRatio = static_cast<float>(Texture->GetWidth()) / static_cast<float>(Texture->GetHeight());
			float HalfWidth = AspectRatio * 0.5f;
			float HalfHeight = 0.5f;

			BillboardBuffer.TextureHalfHeight = HalfHeight;
			BillboardBuffer.TextureHalfWidth = HalfWidth;
		}
	}

	// Set Scale
	if (bIsScaleDirty)
	{
		bIsScaleDirty = false;

		FVector Scale = GetRelativeScale();
		float AbsoluteScale = std::max({fabs(Scale.X), fabs(Scale.Y), fabs(Scale.Z), 1.0f});
		BillboardBuffer.Scale = AbsoluteScale;
	}
	
	// Set Billboard Variant
	BillboardBuffer.InverseViewMat = View.InverseAffine();

	// Set Billboard Location
	FVector CompPosition = GetWorldLocation() + FVector(0.f, 0.f, 1.f) * Owner->GetActorScale().Z;
	BillboardBuffer.WorldPos = CompPosition;
	
	Renderer->UpdateBillboardConstantBuffers(BillboardBuffer);
	Renderer->PrepareShader(Material->GetShader());
	Renderer->OMSetBlendState(true);
	Renderer->RSSetState(EViewModeIndex::VMI_Unlit);
	Renderer->DrawIndexedPrimitiveComponent(this, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Renderer->OMSetBlendState(false);
}



