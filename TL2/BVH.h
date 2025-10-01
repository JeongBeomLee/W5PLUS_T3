#pragma once
#include "Vector.h"
#include "UEContainer.h"
#include"AABoundingBoxComponent.h"
#include "Actor.h"
#include <cmath>

struct FBound;

// 최적화된 Ray-AABB 교차 검사를 위한 구조체
struct alignas(16) FOptimizedRay
{
    FVector Origin;
    FVector Direction;
    FVector InverseDirection;  // 1.0f / Direction (division 제거용)
    int Sign[3];              // Direction의 부호 (branchless용)

    FOptimizedRay(const FVector& InOrigin, const FVector& InDirection)
        : Origin(InOrigin), Direction(InDirection)
    {
        // InverseDirection 계산 (0으로 나누기 방지)
        InverseDirection.X = (std::abs(InDirection.X) < KINDA_SMALL_NUMBER) ?
                            (InDirection.X < 0.0f ? -1e30f : 1e30f) : 1.0f / InDirection.X;
        InverseDirection.Y = (std::abs(InDirection.Y) < KINDA_SMALL_NUMBER) ?
                            (InDirection.Y < 0.0f ? -1e30f : 1e30f) : 1.0f / InDirection.Y;
        InverseDirection.Z = (std::abs(InDirection.Z) < KINDA_SMALL_NUMBER) ?
                            (InDirection.Z < 0.0f ? -1e30f : 1e30f) : 1.0f / InDirection.Z;

        // Sign 계산 (branchless slab method용)
        Sign[0] = (InverseDirection.X < 0.0f) ? 1 : 0;
        Sign[1] = (InverseDirection.Y < 0.0f) ? 1 : 0;
        Sign[2] = (InverseDirection.Z < 0.0f) ? 1 : 0;
    }

    // 최적화된 Ray-AABB 교차 검사 (branchless slab method)
    inline bool IntersectAABB(const FBound& Box, float& OutTNear) const
    {
        // AABB의 min/max를 배열로 접근하기 위한 설정
        FVector BoundsArray[2] = { Box.Min, Box.Max };

        float tmin = (BoundsArray[Sign[0]].X - Origin.X) * InverseDirection.X;
        float tmax = (BoundsArray[1 - Sign[0]].X - Origin.X) * InverseDirection.X;

        float tymin = (BoundsArray[Sign[1]].Y - Origin.Y) * InverseDirection.Y;
        float tymax = (BoundsArray[1 - Sign[1]].Y - Origin.Y) * InverseDirection.Y;

        // Branch 없이 min/max 계산
        tmin = FMath::Max(tmin, tymin);
        tmax = FMath::Min(tmax, tymax);

        float tzmin = (BoundsArray[Sign[2]].Z - Origin.Z) * InverseDirection.Z;
        float tzmax = (BoundsArray[1 - Sign[2]].Z - Origin.Z) * InverseDirection.Z;

        tmin = FMath::Max(tmin, tzmin);
        tmax = FMath::Min(tmax, tzmax);

        // 교차 여부 및 거리 반환
        OutTNear = tmin;
        return (tmax >= tmin) && (tmax >= 0.0f);
    }
};

// BVH 노드 구조체
struct FBVHNode
{
    FBound BoundingBox;

    // 리프 노드용 데이터
    int FirstActor;  // 리프인 경우: 첫 번째 액터 인덱스, 내부 노드인 경우: -1
    int ActorCount;  // 리프인 경우: 액터 개수, 내부 노드인 경우: -1

    // 내부 노드용 데이터
    int LeftChild;   // 왼쪽 자식 노드 인덱스
    int RightChild;  // 오른쪽 자식 노드 인덱스

    // 생성자
    FBVHNode()
        : FirstActor(-1), ActorCount(-1), LeftChild(-1), RightChild(-1)
    {
    }

    // 리프 노드인지 확인
    bool IsLeaf() const { return FirstActor >= 0 && ActorCount > 0; }
};

// 액터의 AABB와 포인터를 저장
struct FActorBounds
{
    FBound Bounds;
    AActor* Actor;
    FVector Center;

    FActorBounds() : Actor(nullptr) {}
    FActorBounds(AActor* InActor, const FBound& InBounds)
        : Actor(InActor), Bounds(InBounds)
    {
        Center = (InBounds.Min + InBounds.Max) * 0.5f;
    }
};

// 고성능 BVH 구현
class FBVH
{
public:
    FBVH();
    ~FBVH();

    // 액터 배열로부터 BVH 구축
    void Build(const TArray<AActor*>& Actors);
    void Clear();

    // 빠른 레이 교차 검사 - 가장 가까운 액터 반환
    AActor* Intersect(const FVector& RayOrigin, const FVector& RayDirection, float& OutDistance) const;

    // 통계 정보
    int GetNodeCount() const { return Nodes.Num(); }
    int GetActorCount() const { return ActorBounds.Num(); }
    int GetMaxDepth() const { return MaxDepth; }

    // 렌더링을 위한 노드 접근
    const TArray<FBVHNode>& GetNodes() const { return Nodes; }

    static float SurfaceArea(const FBound& b);
private:
    TArray<FBVHNode> Nodes;
    TArray<FActorBounds> ActorBounds;
    TArray<int> ActorIndices; // 정렬된 액터 인덱스

    int MaxDepth;

    // 재귀 구축 함수
    int BuildRecursive(int FirstActor, int ActorCount, int Depth = 0);

    // 경계 박스 계산
    FBound CalculateBounds(int FirstActor, int ActorCount) const;
    FBound CalculateCentroidBounds(int FirstActor, int ActorCount) const;

    // Surface Area Heuristic을 이용한 최적 분할
    int FindBestSplit(int FirstActor, int ActorCount, int& OutAxis, float& OutSplitPos);
    float CalculateSAH(int FirstActor, int LeftCount, int RightCount, const FBound& ParentBounds) const;

    // 액터 분할
    int PartitionActors(int FirstActor, int ActorCount, int Axis, float SplitPos);

    bool IntersectNode(int NodeIndex, const FOptimizedRay& Ray, float& InOutDistance, AActor*& OutActor) const;

    //// 재귀 교차 검사 (깊이 제한 추가)
    //bool IntersectNode(int NodeIndex, const FVector& RayOrigin, const FVector& RayDirection,
    //                   float& InOutDistance, AActor*& OutActor, int RecursionDepth = 0) const;

    // 액터와의 교차 검사
    bool IntersectActor(AActor* Actor, const FVector& RayOrigin, const FVector& RayDirection,
                        float& OutDistance) const;

    // 상수
    static const int MaxActorsPerLeaf = 8;  // 리프당 최대 액터 수 (마이크로 BVH용)
    static const int MaxBVHDepth = 24;      // 최대 깊이
    static const int SAHSamples = 10;       // SAH 샘플링 수
};