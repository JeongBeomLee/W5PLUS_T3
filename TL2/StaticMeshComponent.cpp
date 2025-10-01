#include "pch.h"
#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include "Shader.h"
#include "Texture.h"
#include "ResourceManager.h"
#include "ObjManager.h"
#include "SceneLoader.h"

UStaticMeshComponent::UStaticMeshComponent()
{
    SetMaterial("StaticMeshShader.hlsl", EVertexLayoutType::PositionColorTexturNormal);
}

UStaticMeshComponent::~UStaticMeshComponent()
{

}

void UStaticMeshComponent::Render(URenderer* Renderer, const FMatrix& ViewMatrix, const FMatrix& ProjectionMatrix)
{
    Renderer->UpdateConstantBuffer(GetWorldMatrix(), ViewMatrix, ProjectionMatrix);
    Renderer->PrepareShader(GetMaterial()->GetShader());
    Renderer->DrawIndexedPrimitiveComponent(GetStaticMesh(), D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST, MaterailSlots);
}

void UStaticMeshComponent::SetStaticMesh(const FString& PathFileName)
{
	StaticMesh = FObjManager::LoadObjStaticMesh(PathFileName);
    
    const TArray<FGroupInfo>& GroupInfos = StaticMesh->GetMeshGroupInfo();
    if (MaterailSlots.size() < GroupInfos.size())
    {
        MaterailSlots.resize(GroupInfos.size());
    }

    // MaterailSlots.size()가 GroupInfos.size() 보다 클 수 있기 때문에, GroupInfos.size()로 설정
    for (int i = 0; i < GroupInfos.size(); ++i) 
    {
        if (MaterailSlots[i].bChangedByUser == false)
        {
            MaterailSlots[i].MaterialName = GroupInfos[i].InitialMaterialName;
        }
    }
}

void UStaticMeshComponent::Serialize(bool bIsLoading, FPrimitiveData& InOut)
{
    // 0) 트랜스폼 직렬화/역직렬화는 상위(UPrimitiveComponent)에서 처리
    UPrimitiveComponent::Serialize(bIsLoading, InOut);

    if (bIsLoading)
    {
        // 1) 신규 포맷: ObjStaticMeshAsset가 있으면 우선 사용
        if (!InOut.ObjStaticMeshAsset.empty())
        {
            SetStaticMesh(InOut.ObjStaticMeshAsset);
            return;
        }

        // 2) 레거시 호환: Type을 "Data/<Type>.obj"로 매핑
        if (!InOut.Type.empty())
        {
            const FString LegacyPath = "Data/" + InOut.Type + ".obj";
            SetStaticMesh(LegacyPath);
        }
    }
    else
    {
        // 저장 시: 현재 StaticMesh가 있다면 실제 에셋 경로를 기록
        if (UStaticMesh* Mesh = GetStaticMesh())
        {
            InOut.ObjStaticMeshAsset = Mesh->GetAssetPathFileName();
        }
        else
        {
            InOut.ObjStaticMeshAsset.clear();
        }
        // Type은 상위(월드/액터) 정책에 따라 별도 기록 (예: "StaticMeshComp")
    }
}

void UStaticMeshComponent::SetMaterialByUser(const uint32 InMaterialSlotIndex, const FString& InMaterialName)
{
    assert((0 <= InMaterialSlotIndex && InMaterialSlotIndex < MaterailSlots.size()) && "out of range InMaterialSlotIndex");

    if (0 <= InMaterialSlotIndex && InMaterialSlotIndex < MaterailSlots.size())
    {
        MaterailSlots[InMaterialSlotIndex].MaterialName = InMaterialName;
        MaterailSlots[InMaterialSlotIndex].bChangedByUser = true;
    }
    else
    {
        UE_LOG("out of range InMaterialSlotIndex: %d", InMaterialSlotIndex);
    }

    assert(MaterailSlots[InMaterialSlotIndex].bChangedByUser == true);
}

UObject* UStaticMeshComponent::Duplicate()
{
    // 부모 클래스의 Duplicate 호출 (Transform + Material 복사)
    UStaticMeshComponent* NewComponent = static_cast<UStaticMeshComponent*>(Super_t::Duplicate());

    if (!NewComponent)
    {
        return nullptr;
    }

    // StaticMesh 포인터 복사 (리소스는 공유)
    NewComponent->StaticMesh = this->StaticMesh;

    // MaterailSlots 복사
    NewComponent->MaterailSlots = this->MaterailSlots;

    return NewComponent;
}


void UStaticMeshComponent::RenderDetail()
{
	UMeshComponent::RenderDetail();

	if (ImGui::TreeNode("StaticMesh"))
	{
		// 현재 메시 경로 표시
		FString CurrentPath;
		UStaticMesh* CurMesh = GetStaticMesh();
		if (CurMesh)
		{
			CurrentPath = CurMesh->GetAssetPathFileName();
			ImGui::Text("Path: %s", CurrentPath.c_str());
		}
		else
		{
			ImGui::Text("Path: <None>");
		}

		// 리소스 매니저에서 로드된 모든 StaticMesh 경로 수집
		auto& RM = UResourceManager::GetInstance();
		TArray<FString> Paths = RM.GetAllStaticMeshFilePaths();

		if (Paths.empty())
		{
			ImGui::TextColored(ImVec4(1, 0.6f, 0.6f, 1), "No StaticMesh resources loaded.");
		}
		else
		{
			// 표시용 이름(파일명 스템)
			TArray<FString> DisplayNames;
			DisplayNames.reserve(Paths.size());
			for (const FString& p : Paths)
				DisplayNames.push_back(GetBaseNameNoExt(p));

			// ImGui 콤보 아이템 배열
			TArray<const char*> Items;
			Items.reserve(DisplayNames.size());
			for (const FString& n : DisplayNames)
				Items.push_back(n.c_str());

			// 선택 인덱스 유지
			static int SelectedMeshIdx = -1;

			// 기본 선택: Cube가 있으면 자동 선택
			if (SelectedMeshIdx == -1)
			{
				for (int i = 0; i < static_cast<int>(Paths.size()); ++i)
				{
					if (DisplayNames[i] == "Cube" || Paths[i] == "Data/Cube.obj")
					{
						SelectedMeshIdx = i;
						break;
					}
				}
			}

			ImGui::SetNextItemWidth(240);
			ImGui::Combo("StaticMesh", &SelectedMeshIdx, Items.data(), static_cast<int>(Items.size()));
			ImGui::SameLine();
			if (ImGui::Button("Apply Mesh"))
			{
				if (SelectedMeshIdx >= 0 && SelectedMeshIdx < static_cast<int>(Paths.size()))
				{
					const FString& NewPath = Paths[SelectedMeshIdx];
					SetStaticMesh(NewPath);

					UE_LOG("Applied StaticMesh: %s", NewPath.c_str());
				}
			}

			// 현재 메시로 선택 동기화 버튼 (옵션)
			ImGui::SameLine();
			if (ImGui::Button("Select Current"))
			{
				SelectedMeshIdx = -1;
				if (!CurrentPath.empty())
				{
					for (int i = 0; i < static_cast<int>(Paths.size()); ++i)
					{
						if (Paths[i] == CurrentPath ||
							DisplayNames[i] == GetBaseNameNoExt(CurrentPath))
						{
							SelectedMeshIdx = i;
							break;
						}
					}
				}
			}
		}

		// Material 설정
		ImGui::Separator();

		const TArray<FString> MaterialNames = UResourceManager::GetInstance().GetAllFilePaths<UMaterial>();
		// ImGui 콤보 아이템 배열
		TArray<const char*> MaterialNamesCharP;
		MaterialNamesCharP.reserve(MaterialNames.size());
		for (const FString& n : MaterialNames)
			MaterialNamesCharP.push_back(n.c_str());

		if (CurMesh)
		{
			const uint64 MeshGroupCount = CurMesh->GetMeshGroupCount();

			static TArray<int32> SelectedMaterialIdxAt; // i번 째 Material Slot이 가지고 있는 MaterialName이 MaterialNames의 몇번쩨 값인지.
			if (SelectedMaterialIdxAt.size() < MeshGroupCount)
			{
				SelectedMaterialIdxAt.resize(MeshGroupCount);
			}

			// 현재 SMC의 MaterialSlots 정보를 UI에 반영
			const TArray<FMaterialSlot>& MaterialSlots = GetMaterailSlots();
			for (uint64 MaterialSlotIndex = 0; MaterialSlotIndex < MeshGroupCount; ++MaterialSlotIndex)
			{
				for (uint32 MaterialIndex = 0; MaterialIndex < MaterialNames.size(); ++MaterialIndex)
				{
					if (MaterialSlots[MaterialSlotIndex].MaterialName == MaterialNames[MaterialIndex])
					{
						SelectedMaterialIdxAt[MaterialSlotIndex] = MaterialIndex;
					}
				}
			}

			// Material 선택
			for (uint64 MaterialSlotIndex = 0; MaterialSlotIndex < MeshGroupCount; ++MaterialSlotIndex)
			{
				ImGui::PushID(static_cast<int>(MaterialSlotIndex));
				if (ImGui::Combo("Material", &SelectedMaterialIdxAt[MaterialSlotIndex], MaterialNamesCharP.data(), static_cast<int>(MaterialNamesCharP.size())))
				{
					SetMaterialByUser(static_cast<uint32>(MaterialSlotIndex), MaterialNames[SelectedMaterialIdxAt[MaterialSlotIndex]]);
				}
				ImGui::PopID();
			}
		}
		ImGui::TreePop();
	}
}