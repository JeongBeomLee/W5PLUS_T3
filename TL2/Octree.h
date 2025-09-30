#pragma once
#include "Object.h"
#include"OBoundingBoxComponent.h"
#include"AABoundingBoxComponent.h"
#include"BVH.h"

struct FOctreeNode
{
    FBound Bounds;                       // 이 노드의 공간 범위
    TArray<AActor*> Actors;            // 이 범위 안의 Actor들
    FOctreeNode* Children[8] = { nullptr }; // 자식 노드 (최대 8개)

    UAABoundingBoxComponent* AABoundingBoxComponent;

    // === Per-Leaf 마이크로 BVH ===
    FBVH* MicroBVH = nullptr;          // 리프 노드에만 사용
    bool bBVHDirty = true;             // BVH 재구축 필요 여부

    FOctreeNode(const FBound& InBounds)
        : Bounds(InBounds)
    {
        AABoundingBoxComponent = NewObject<UAABoundingBoxComponent>();
        AABoundingBoxComponent->SetMinMax(Bounds);
    }

    ~FOctreeNode()
    {
        if (MicroBVH)
        {
            delete MicroBVH;
            MicroBVH = nullptr;
        }
    }

    bool IsLeafNode() const
    {
        for (int i = 0; i < 8; ++i)
        {
            if (Children[i] != nullptr)
                return false;
        }
        return true;
    }

    // === 마이크로 BVH 관리 ===
    void EnsureMicroBVH()
    {
        if (IsLeafNode() && Actors.Num() > 0)
        {
            if (!MicroBVH)
            {
                MicroBVH = new FBVH();
                bBVHDirty = true;
            }

            if (bBVHDirty)
            {
                MicroBVH->Clear();
                MicroBVH->Build(Actors);
                bBVHDirty = false;
            }
        }
    }

    void MarkDirty()
    {
        bBVHDirty = true;
    }
};


class UOctree:public UObject
{
  
public:
    DECLARE_CLASS(UOctree, UObject)
    UOctree();
    ~UOctree();
    void Initialize(const FBound& InBounds);
    // 옥트리 빌드
    void Build(const TArray<AActor*>& InActors, const FBound& WorldBounds, int32 Depth = 0);

    // 레이 기반 Actor 검색
    void Query(const FRay& Ray, TArray<AActor*>& OutActors) const;

    // 빌드 타임에 모든 리프 노드의 마이크로 BVH 미리 생성
    void PreBuildAllMicroBVH();

    void Render(FOctreeNode* Root);

    // BVH 렌더링
    void RenderBVH(FBVH* BVH);

    // 전체 정리
    void Release();

private:
    FOctreeNode* Root = nullptr;
    int32 MaxDepth = 5;//최대 깊이 조절해봐야하고
    // Leaf Node에 존재할 수 있는 최대 액터 수
    int32 MaxActorsPerNode = 8;//노드당 최소 엑터수 이거 이하여야만 끝나는 거 

    // 내부 함수들
    /**
    * @brief 자식 노드를 재귀적으로 생성합니다.
    * @param InActors 액터 배열
    * @param Bounds 해당 노드가 가진 범위
    * @param Depth 트리의 현재 깊이
    */
    void BuildRecursive(FOctreeNode* ChildNode, const TArray<AActor*>& InActors, const FBound& Bounds, int32 Depth);
    /**
    * @brief Ray와 Node의 Bound의 충돌 여부를 재귀적으로 검사해서 해당 공간의 Actors를 쿼리합니다.
    * @param Ray 발사할 Ray를 지정합니다.
    * @param Node 탐색 중인 현재 노드입니다.
    * @param OutActors 쿼리된 Actors가 저장됩니다.
    */
    void QueryRecursive(const FRay& Ray, FOctreeNode* Node, TArray<AActor*>& OutActors) const;
    /**
    * @breif 옥트리의 Node를 재귀적으로 해제합니다.
    * @param Node 해제할 현재 Node
    */
    void ReleaseRecursive(FOctreeNode* Node);

    // 재귀적으로 모든 리프 노드의 마이크로 BVH 생성
    void PreBuildMicroBVHRecursive(FOctreeNode* Node);

    // 리프 노드 통계 로그
    void LogLeafNodeStatistics();
    void LogLeafNodeStatisticsRecursive(FOctreeNode* Node, int32& LeafCount, int32& TotalActors, int32& MinActors, int32& MaxActors, int32 Depth, int32& MaxDepthFound);
};