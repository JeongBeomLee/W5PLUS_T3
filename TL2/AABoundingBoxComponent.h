#pragma once
#include "ShapeComponent.h"

struct FBox;
struct FBound
{
    FVector Min;
    FVector Max;

    FBound() : Min(FVector()), Max(FVector()) {}
    FBound(const FVector& InMin, const FVector& InMax) : Min(InMin), Max(InMax) {}
    /**
     * @brief 다른 FBound를 현재 FBound에 포함시키도록 확장하는 연산자
     * @param Other 포함시킬 다른 FBound
     * @return 확장된 자기 자신에 대한 참조
     */
    FBound& operator+=(const FBound& Other)
    {
        Min.X = std::min(Min.X, Other.Min.X);
        Min.Y = std::min(Min.Y, Other.Min.Y);
        Min.Z = std::min(Min.Z, Other.Min.Z);

        Max.X = std::max(Max.X, Other.Max.X);
        Max.Y = std::max(Max.Y, Other.Max.Y);
        Max.Z = std::max(Max.Z, Other.Max.Z);

        return *this;
    }
    // 센터 반환
    FVector GetCenter() const
    {
        return (Min + Max) * 0.5f;
    }
    // 박스의 절반 크기
    FVector GetExtent() const
    {
        return (Max - Min) * 0.5f;
    }

    // 표면적 계산 (SAH용)
    float GetSurfaceArea() const
    {
        FVector size = Max - Min;
        if (size.X <= 0.0f || size.Y <= 0.0f || size.Z <= 0.0f) return 0.0f;
        return 2.0f * (size.X * size.Y + size.Y * size.Z + size.Z * size.X);
    }
    bool IsInside(const FVector& Point) const
    {
        return (Point.X >= Min.X && Point.X <= Max.X &&
            Point.Y >= Min.Y && Point.Y <= Max.Y &&
            Point.Z >= Min.Z && Point.Z <= Max.Z);
    }
    bool IsIntersect(const FBound& Other) const
    {
        return (Min.X <= Other.Max.X && Max.X >= Other.Min.X &&
            Min.Y <= Other.Max.Y && Max.Y >= Other.Min.Y &&
            Min.Z <= Other.Max.Z && Max.Z >= Other.Min.Z);
    }

    bool RayIntersects(const FVector& Origin, const FVector& Direction, float& Distance) const
    {
        FVector invDir = FVector(1.0f / Direction.X, 1.0f / Direction.Y, 1.0f / Direction.Z);

        FVector t1 = (Min - Origin) * invDir;
        FVector t2 = (Max - Origin) * invDir;

        FVector tMin = t1.ComponentMin(t2);
        FVector tMax = t1.ComponentMax(t2);

        float tNear = std::max({tMin.X, tMin.Y, tMin.Z});
        float tFar = std::min({tMax.X, tMax.Y, tMax.Z});

        if (tNear > tFar || tFar < 0.0f) return false;

        Distance = (tNear < 0.0f) ? 0.0f : tNear;
        return true;
    }
};

struct FOrientedBound
{
    FVector Center;
    FVector Extents;
    FMatrix Orientation;

    FOrientedBound() : Center(FVector()), Extents(FVector()), Orientation(FMatrix::Identity()) {}
    FOrientedBound(const FVector& InCenter, const FVector& InExtents, const FMatrix& InOrientation)
        : Center(InCenter), Extents(InExtents), Orientation(InOrientation) {}

    bool RayIntersects(const FVector& Origin, const FVector& Direction, float& Distance) const
    {
        const float Epsilon = 1e-6f;

        FVector RelativeOrigin = Center - Origin;

        FVector AxisX = FVector(Orientation.M[0][0], Orientation.M[1][0], Orientation.M[2][0]);
        FVector AxisY = FVector(Orientation.M[0][1], Orientation.M[1][1], Orientation.M[2][1]);
        FVector AxisZ = FVector(Orientation.M[0][2], Orientation.M[1][2], Orientation.M[2][2]);

        float AxisDotOrigin[3] = {
            AxisX.Dot(RelativeOrigin),
            AxisY.Dot(RelativeOrigin),
            AxisZ.Dot(RelativeOrigin)
        };

        float AxisDotDirection[3] = {
            AxisX.Dot(Direction),
            AxisY.Dot(Direction),
            AxisZ.Dot(Direction)
        };

        float tMin = -FLT_MAX;
        float tMax = FLT_MAX;

        for (int i = 0; i < 3; ++i)
        {
            float extent = (i == 0) ? Extents.X : (i == 1) ? Extents.Y : Extents.Z;

            if (std::abs(AxisDotDirection[i]) <= Epsilon)
            {
                if (std::abs(AxisDotOrigin[i]) > extent)
                    return false;
            }
            else
            {
                float invAxisDotDirection = 1.0f / AxisDotDirection[i];
                float t1 = (AxisDotOrigin[i] - extent) * invAxisDotDirection;
                float t2 = (AxisDotOrigin[i] + extent) * invAxisDotDirection;

                if (t1 > t2) std::swap(t1, t2);

                tMin = std::max(tMin, t1);
                tMax = std::min(tMax, t2);

                if (tMin > tMax) return false;
            }
        }

        if (tMax < 0.0f) return false;

        Distance = tMin >= 0.0f ? tMin : tMax;
        return true;
    }
};

class ULine;
class UAABoundingBoxComponent :
    public UShapeComponent
{
    DECLARE_CLASS(UAABoundingBoxComponent, UShapeComponent)
public:
    UAABoundingBoxComponent();

    // 주어진 로컬 버텍스들로부터 Min/Max 계산
    void SetFromVertices(const TArray<FVector>& Verts);
    void SetFromVertices(const TArray<FNormalVertex>& Verts);
    void SetMinMax(const FBound& Bound);
    void Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj) override;

    // 월드 좌표계에서의 AABB 반환
    FBound GetWorldBoundFromCube() const;
    FBound GetWorldBoundFromSphere() const;
    const FBound* GetFBound() const;
    // OBB 관련 함수들
    FOrientedBound GetWorldOrientedBound() const;
    bool RayIntersectsOBB(const FVector& Origin, const FVector& Direction, float& Distance) const;

    TArray<FVector4> GetLocalCorners() const;

    void SetPrimitiveType(EPrimitiveType InType) { PrimitiveType = InType; }
    void SetLineColor(FVector4 InLineColor) { LineColor = InLineColor; };
private:
    void CreateLineData(
        const FVector& Min, const FVector& Max,
        OUT TArray<FVector>& Start,
        OUT TArray<FVector>& End,
        OUT TArray<FVector4>& Color);

    FBound Bound;
    FVector LocalMin;
    FVector LocalMax;
    FVector4 LineColor;
    EPrimitiveType PrimitiveType = EPrimitiveType::Default;
};

