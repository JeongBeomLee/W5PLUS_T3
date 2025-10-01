#include "pch.h"
#include "BVH.h"
#include "StaticMeshActor.h"
#include "Picking.h"
#include "PickingTimer.h"
#include "UI/GlobalConsole.h"
#include <algorithm>
#include <cfloat>


FBVH::FBVH() : MaxDepth(0)
{
}

FBVH::~FBVH()
{
    Clear();
}

void FBVH::Build(const TArray<AActor*>& Actors)
{
    TStatId BVHBuildStatId;
    FScopeCycleCounter BVHBuildTimer(BVHBuildStatId);

    Clear();

    if (Actors.Num() == 0)
        return;

    // 1. 액터들의 AABB 정보 수집
    ActorBounds.Reserve(Actors.Num());
    ActorIndices.Reserve(Actors.Num());

    for (int i = 0; i < Actors.Num(); ++i)
    {
        AActor* Actor = Actors[i];
        if (!Actor || Actor->GetActorHiddenInGame())
            continue;

        const FBound* ActorBounds_Local = nullptr;
        bool bHasBounds = false;

        if (const AStaticMeshActor* StaticMeshActor = Cast<const AStaticMeshActor>(Actor))
        {
            for (auto Component : StaticMeshActor->GetAllComponents()) // 최적화: AABB 컴포넌트만 검색
            {
                if (UAABoundingBoxComponent* AABBComponent = Cast<UAABoundingBoxComponent>(Component))
                {
                    ActorBounds_Local = AABBComponent->GetFBound();
                    bHasBounds = true;
                    break;
                }
            }
        }

        if (bHasBounds)
        {
            FActorBounds AB(Actor, *ActorBounds_Local);
            ActorBounds.Add(AB);
            ActorIndices.Add(ActorBounds.Num() - 1);
        }
    }

    if (ActorBounds.Num() == 0)
        return;

    // 2. 노드 배열 예약 (최악의 경우 2*N-1개 노드)
    Nodes.Reserve(ActorBounds.Num() * 2);

    // 3. 재귀적으로 BVH 구축
    MaxDepth = 0;
    int RootIndex = BuildRecursive(0, ActorBounds.Num(), 0);

    uint64_t BuildCycles = BVHBuildTimer.Finish();
    double BuildTimeMs = FPlatformTime::ToMilliseconds(BuildCycles);

    char buf[256];
    sprintf_s(buf, "[BVH] Built for %d actors, %d nodes, depth %d (Time: %.3fms)\n",
        ActorBounds.Num(), Nodes.Num(), MaxDepth, BuildTimeMs);
    UE_LOG(buf);
}

void FBVH::Clear()
{
    Nodes.Empty();
    ActorBounds.Empty();
    ActorIndices.Empty();
    MaxDepth = 0;
}

AActor* FBVH::Intersect(const FVector& RayOrigin, const FVector& RayDirection, float& OutDistance) const
{
    if (Nodes.Num() == 0)
        return nullptr;

    TStatId BVHIntersectStatId;
    FScopeCycleCounter BVHIntersectTimer(BVHIntersectStatId);

    OutDistance = FLT_MAX;
    AActor* HitActor = nullptr;

    // 최적화된 Ray 생성 (InverseDirection과 Sign 미리 계산)
    FOptimizedRay OptRay(RayOrigin, RayDirection);
    bool bHit = IntersectNode(0, OptRay, OutDistance, HitActor);

    uint64_t IntersectCycles = BVHIntersectTimer.Finish();
    double IntersectTimeMs = FPlatformTime::ToMilliseconds(IntersectCycles);

    if (bHit)
    {
        char buf[256];
        sprintf_s(buf, "[BVH Pick] Hit actor at distance %.3f (Time: %.3fms)\n",
            OutDistance, IntersectTimeMs);
        UE_LOG(buf);
        return HitActor;
    }
    else
    {
        char buf[256];
        sprintf_s(buf, "[BVH Pick] No hit (Time: %.3fms)\n", IntersectTimeMs);
        UE_LOG(buf);
        return nullptr;
    }
}

int FBVH::BuildRecursive(int FirstActor, int ActorCount, int Depth)
{
    MaxDepth = FMath::Max(MaxDepth, Depth);

    int NodeIndex = Nodes.Num();
    FBVHNode NewNode;
    Nodes.Add(NewNode);
    FBVHNode& Node = Nodes[NodeIndex];

    Node.BoundingBox = CalculateBounds(FirstActor, ActorCount);

    if (ActorCount <= MaxActorsPerLeaf || Depth >= MaxBVHDepth)
    {
        Node.FirstActor = FirstActor;
        Node.ActorCount = ActorCount;
        return NodeIndex;
    }

    int BestAxis;
    float BestSplitPos;
    int SplitIndex = FindBestSplit(FirstActor, ActorCount, BestAxis, BestSplitPos);

    if (SplitIndex == FirstActor || SplitIndex == FirstActor + ActorCount)
    {
        Node.FirstActor = FirstActor;
        Node.ActorCount = ActorCount;
        return NodeIndex;
    }

    int ActualSplit = PartitionActors(FirstActor, ActorCount, BestAxis, BestSplitPos);

    int LeftCount = ActualSplit - FirstActor;
    int RightCount = ActorCount - LeftCount;

    if (LeftCount == 0 || RightCount == 0)
    {
        Node.FirstActor = FirstActor;
        Node.ActorCount = ActorCount;
        return NodeIndex;
    }

    Node.LeftChild = BuildRecursive(FirstActor, LeftCount, Depth + 1);
    Node.RightChild = BuildRecursive(ActualSplit, RightCount, Depth + 1);

    return NodeIndex;
}
// BVH.cpp
float FBVH::SurfaceArea(const FBound& b) {
    FVector s = b.Max - b.Min;
    if (s.X <= 0 || s.Y <= 0 || s.Z <= 0) return 0.0f;
    return 2.0f * (s.X * s.Y + s.Y * s.Z + s.Z * s.X);
}

FBound FBVH::CalculateBounds(int FirstActor, int ActorCount) const
{
    FBound Bounds;
    bool bFirst = true;

    for (int i = 0; i < ActorCount; ++i)
    {
        int ActorIndex = ActorIndices[FirstActor + i];
        const FBound& ActorBound = ActorBounds[ActorIndex].Bounds;

        if (bFirst)
        {
            Bounds = ActorBound;
            bFirst = false;
        }
        else
        {
            Bounds.Min.X = FMath::Min(Bounds.Min.X, ActorBound.Min.X);
            Bounds.Min.Y = FMath::Min(Bounds.Min.Y, ActorBound.Min.Y);
            Bounds.Min.Z = FMath::Min(Bounds.Min.Z, ActorBound.Min.Z);

            Bounds.Max.X = FMath::Max(Bounds.Max.X, ActorBound.Max.X);
            Bounds.Max.Y = FMath::Max(Bounds.Max.Y, ActorBound.Max.Y);
            Bounds.Max.Z = FMath::Max(Bounds.Max.Z, ActorBound.Max.Z);
        }
    }

    return Bounds;
}

FBound FBVH::CalculateCentroidBounds(int FirstActor, int ActorCount) const
{
    FBound Bounds;
    bool bFirst = true;

    for (int i = 0; i < ActorCount; ++i)
    {
        int ActorIndex = ActorIndices[FirstActor + i];
        const FVector& Center = ActorBounds[ActorIndex].Center;

        if (bFirst)
        {
            Bounds.Min = Bounds.Max = Center;
            bFirst = false;
        }
        else
        {
            Bounds.Min.X = FMath::Min(Bounds.Min.X, Center.X);
            Bounds.Min.Y = FMath::Min(Bounds.Min.Y, Center.Y);
            Bounds.Min.Z = FMath::Min(Bounds.Min.Z, Center.Z);

            Bounds.Max.X = FMath::Max(Bounds.Max.X, Center.X);
            Bounds.Max.Y = FMath::Max(Bounds.Max.Y, Center.Y);
            Bounds.Max.Z = FMath::Max(Bounds.Max.Z, Center.Z);
        }
    }

    return Bounds;
}

// 🔥 새로 추가: Bound 합치는 유틸리티
static inline FBound Union(const FBound& A, const FBound& B)
{
    FBound Out;
    Out.Min.X = FMath::Min(A.Min.X, B.Min.X);
    Out.Min.Y = FMath::Min(A.Min.Y, B.Min.Y);
    Out.Min.Z = FMath::Min(A.Min.Z, B.Min.Z);

    Out.Max.X = FMath::Max(A.Max.X, B.Max.X);
    Out.Max.Y = FMath::Max(A.Max.Y, B.Max.Y);
    Out.Max.Z = FMath::Max(A.Max.Z, B.Max.Z);
    return Out;
}

int FBVH::FindBestSplit(int FirstActor, int ActorCount, int& OutAxis, float& OutSplitPos)
{
    FBound CentroidBounds = CalculateCentroidBounds(FirstActor, ActorCount);
    FBound ParentBounds = CalculateBounds(FirstActor, ActorCount);

    FVector Extent = CentroidBounds.Max - CentroidBounds.Min;
    OutAxis = 0;
    if (Extent.Y > Extent.X) OutAxis = 1;
    if (Extent.Z > Extent[OutAxis]) OutAxis = 2;

    if (Extent[OutAxis] < KINDA_SMALL_NUMBER)
    {
        OutSplitPos = CentroidBounds.Min[OutAxis];
        return FirstActor + ActorCount / 2;
    }

    // 1) 정렬 - TArray의 Sort 사용
    // 임시 배열 생성 후 정렬
    TArray<int> TempIndices;
    TempIndices.Reserve(ActorCount);
    for (int i = 0; i < ActorCount; ++i)
    {
        TempIndices.Add(ActorIndices[FirstActor + i]);
    }

    TempIndices.Sort([&](int A, int B)
    {
        return ActorBounds[A].Center[OutAxis] < ActorBounds[B].Center[OutAxis];
    });

    // 정렬된 결과를 다시 복사
    for (int i = 0; i < ActorCount; ++i)
    {
        ActorIndices[FirstActor + i] = TempIndices[i];
    }
    // 2) Prefix/Suffix AABB 계산
    TArray<FBound> Prefix;
    TArray<FBound> Suffix;
    Prefix.SetNum(ActorCount);
    Suffix.SetNum(ActorCount);

    Prefix[0] = ActorBounds[ActorIndices[FirstActor]].Bounds;
    for (int i = 1; i < ActorCount; i++)
        Prefix[i] = Union(Prefix[i - 1], ActorBounds[ActorIndices[FirstActor + i]].Bounds);

    Suffix[ActorCount - 1] = ActorBounds[ActorIndices[FirstActor + ActorCount - 1]].Bounds;
    for (int i = ActorCount - 2; i >= 0; i--)
        Suffix[i] = Union(Suffix[i + 1], ActorBounds[ActorIndices[FirstActor + i]].Bounds);

    // 3) SAH 비용 평가
    float BestCost = FLT_MAX;
    int BestSplit = FirstActor + ActorCount / 2;
    float SA_P = SurfaceArea(ParentBounds) + 1e-6f;

    for (int i = 0; i < ActorCount - 1; i++)
    {
        int LeftCount = i + 1;
        int RightCount = ActorCount - LeftCount;

        float SA_L = SurfaceArea(Prefix[i]);
        float SA_R = SurfaceArea(Suffix[i + 1]);

        float Cost = 1.0f + (SA_L / SA_P) * LeftCount + (SA_R / SA_P) * RightCount;

        if (Cost < BestCost)
        {
            BestCost = Cost;
            BestSplit = FirstActor + LeftCount;
            OutSplitPos = ActorBounds[ActorIndices[BestSplit]].Center[OutAxis];
        }
    }

    return BestSplit;
}

//static inline float SurfaceArea(const FBound& b) {
//    FVector s = b.Max - b.Min;
//    if (s.X <= 0 || s.Y <= 0 || s.Z <= 0) return 0.0f;
//    return 2.0f * (s.X * s.Y + s.Y * s.Z + s.Z * s.X);
//}

float FBVH::CalculateSAH(int FirstActor, int LeftCount, int RightCount, const FBound& Parent) const
{
    FBound LB = CalculateBounds(FirstActor, LeftCount);
    FBound RB = CalculateBounds(FirstActor + LeftCount, RightCount);

    float SA_P = SurfaceArea(Parent) + 1e-6f;
    float SA_L = SurfaceArea(LB);
    float SA_R = SurfaceArea(RB);

    constexpr float Ct = 1.0f;
    constexpr float Ci = 1.0f;
    return Ct + Ci * ((SA_L / SA_P) * LeftCount + (SA_R / SA_P) * RightCount);
}

int FBVH::PartitionActors(int FirstActor, int ActorCount, int Axis, float SplitPos)
{
    int Left = FirstActor;
    int Right = FirstActor + ActorCount - 1;

    while (Left <= Right)
    {
        while (Left <= Right)
        {
            int LeftActorIndex = ActorIndices[Left];
            const FVector& LeftCenter = ActorBounds[LeftActorIndex].Center;
            if (LeftCenter[Axis] >= SplitPos)
                break;
            Left++;
        }

        while (Left <= Right)
        {
            int RightActorIndex = ActorIndices[Right];
            const FVector& RightCenter = ActorBounds[RightActorIndex].Center;
            if (RightCenter[Axis] < SplitPos)
                break;
            Right--;
        }

        if (Left < Right)
        {
            int Temp = ActorIndices[Left];
            ActorIndices[Left] = ActorIndices[Right];
            ActorIndices[Right] = Temp;
            Left++;
            Right--;
        }
    }

    return Left;
}

bool FBVH::IntersectNode(int NodeIndex,
    const FOptimizedRay& Ray,
    float& InOutDistance,
    AActor*& OutActor) const
{
    const FBVHNode& Node = Nodes[NodeIndex];

    // 최적화된 Ray-AABB 교차 검사 사용
    float tNear;
    if (!Ray.IntersectAABB(Node.BoundingBox, tNear))
        return false;

    if (tNear >= InOutDistance)
        return false;

    if (Node.IsLeaf())
    {
        bool bHit = false;
        float Closest = InOutDistance;
        AActor* ClosestActor = nullptr;

        for (int i = 0; i < Node.ActorCount; ++i)
        {
            int ActorIndex = ActorIndices[Node.FirstActor + i];
            AActor* Actor = ActorBounds[ActorIndex].Actor;

            float Dist;
            if (IntersectActor(Actor, Ray.Origin, Ray.Direction, Dist))
            {
                if (Dist < Closest)
                {
                    Closest = Dist;
                    ClosestActor = Actor;
                    bHit = true;
                }
            }
        }

        if (bHit)
        {
            InOutDistance = Closest;
            OutActor = ClosestActor;
        }

        return bHit;
    }

    struct ChildHit
    {
        int Index;
        float tNear;
        bool bValid;
    };

    auto TestChild = [&](int ChildIdx) -> ChildHit
        {
            if (ChildIdx < 0) return { ChildIdx, FLT_MAX, false };
            float tN;
            if (Ray.IntersectAABB(Nodes[ChildIdx].BoundingBox, tN))
                return { ChildIdx, tN, true };
            return { ChildIdx, FLT_MAX, false };
        };

    ChildHit L = TestChild(Node.LeftChild);
    ChildHit R = TestChild(Node.RightChild);

    bool bHit = false;

    if (L.bValid && R.bValid)
    {
        const ChildHit First = (L.tNear < R.tNear) ? L : R;
        const ChildHit Second = (L.tNear < R.tNear) ? R : L;

        if (IntersectNode(First.Index, Ray, InOutDistance, OutActor))
            bHit = true;

        if (InOutDistance > Second.tNear)
        {
            if (IntersectNode(Second.Index, Ray, InOutDistance, OutActor))
                bHit = true;
        }
    }
    else if (L.bValid)
    {
        if (IntersectNode(L.Index, Ray, InOutDistance, OutActor))
            bHit = true;
    }
    else if (R.bValid)
    {
        if (IntersectNode(R.Index, Ray, InOutDistance, OutActor))
            bHit = true;
    }

    return bHit;
}

bool FBVH::IntersectActor(AActor* Actor, const FVector& RayOrigin, const FVector& RayDirection,
    float& OutDistance) const
{
    FRay Ray;
    Ray.Origin = RayOrigin;
    Ray.Direction = RayDirection;

    return CPickingSystem::CheckActorPicking(Actor, Ray, OutDistance);
}
