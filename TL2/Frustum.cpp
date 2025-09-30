#include "pch.h"
#include "Frustum.h"

#include "AABoundingBoxComponent.h"
#include "Vector.h"

void FFrustum::Update(const FMatrix& InViewProjectionMatrix)
{
	const FMatrix& M = InViewProjectionMatrix;

	// 행 우선(Row-Major) 공식은 행렬의 '열(Column)'을 조합합니다.

	// Left Plane (column 3 + column 0)
	Planes[0].Normal.X = M.M[0][3] + M.M[0][0];
	Planes[0].Normal.Y = M.M[1][3] + M.M[1][0];
	Planes[0].Normal.Z = M.M[2][3] + M.M[2][0];
	Planes[0].Distance = M.M[3][3] + M.M[3][0];

	// Right Plane (column 3 - column 0)
	Planes[1].Normal.X = M.M[0][3] - M.M[0][0];
	Planes[1].Normal.Y = M.M[1][3] - M.M[1][0];
	Planes[1].Normal.Z = M.M[2][3] - M.M[2][0];
	Planes[1].Distance = M.M[3][3] - M.M[3][0];

	// Bottom Plane (column 3 + column 1)
	Planes[2].Normal.X = M.M[0][3] + M.M[0][1];
	Planes[2].Normal.Y = M.M[1][3] + M.M[1][1];
	Planes[2].Normal.Z = M.M[2][3] + M.M[2][1];
	Planes[2].Distance = M.M[3][3] + M.M[3][1];

	// Top Plane (column 3 - column 1)
	Planes[3].Normal.X = M.M[0][3] - M.M[0][1];
	Planes[3].Normal.Y = M.M[1][3] - M.M[1][1];
	Planes[3].Normal.Z = M.M[2][3] - M.M[2][1];
	Planes[3].Distance = M.M[3][3] - M.M[3][1];

	// Near Plane (column 2) - DirectX z=0..1
	Planes[4].Normal.X = M.M[0][2];
	Planes[4].Normal.Y = M.M[1][2];
	Planes[4].Normal.Z = M.M[2][2];
	Planes[4].Distance = M.M[3][2];

	// Far Plane (column 3 - column 2)
	Planes[5].Normal.X = M.M[0][3] - M.M[0][2];
	Planes[5].Normal.Y = M.M[1][3] - M.M[1][2];
	Planes[5].Normal.Z = M.M[2][3] - M.M[2][2];
	Planes[5].Distance = M.M[3][3] - M.M[3][2];


	// 6개 평면을 모두 정규화(Normalize)합니다. (이 부분은 동일)
	for (int i = 0; i < 6; ++i)
	{
		float NormalLength = sqrt(
			Planes[i].Normal.X * Planes[i].Normal.X +
			Planes[i].Normal.Y * Planes[i].Normal.Y +
			Planes[i].Normal.Z * Planes[i].Normal.Z
		);

		if (NormalLength > 0.0f) // 0으로 나누는 것을 방지
		{
			Planes[i].Normal.X /= NormalLength;
			Planes[i].Normal.Y /= NormalLength;
			Planes[i].Normal.Z /= NormalLength;
			Planes[i].Distance /= NormalLength;
		}
	}
}

/**
 * AABB가 절두체에 의해 잠재적으로 보이는 상태인지 검사합니다.
 *
 * @param InBox 검사할 AABB(Axis-Aligned Bounding Box)입니다.
 * @return AABB가 보이거나 교차하면 true, 완전히 보이지 않아 컬링 대상이면 false를 반환합니다.
 */
bool FFrustum::IsVisible(FBound& InBox) const
{
	// 절두체를 구성하는 6개의 평면에 대해 모두 검사합니다.
	for (int32 i = 0; i < 6; ++i)
	{
		const FPlane& CurrentPlane = Planes[i];

		// AABB의 8개 꼭짓점 중 현재 평면의 안쪽(법선 방향)으로 가장 멀리 있는 꼭짓점,
		// 즉 '긍정적 꼭짓점(Positive Vertex)'을 찾습니다.
		// 이 꼭짓점은 평면의 '안쪽'에 있을 확률이 가장 높습니다.
		FVector PositiveVtx;
		PositiveVtx.X = (CurrentPlane.Normal.X >= 0.0f) ? InBox.Max.X : InBox.Min.X;
		PositiveVtx.Y = (CurrentPlane.Normal.Y >= 0.0f) ? InBox.Max.Y : InBox.Min.Y;
		PositiveVtx.Z = (CurrentPlane.Normal.Z >= 0.0f) ? InBox.Max.Z : InBox.Min.Z;

		// 이 '가장 안쪽에 있을 법한' 꼭짓점과 평면 사이의 부호 있는 거리를 계산합니다.
		const float SignedDistance = FVector::Dot(CurrentPlane.Normal, PositiveVtx) + CurrentPlane.Distance;

		// 만약 이 꼭짓점마저 평면의 바깥쪽(음수 공간)에 있다면,
		// AABB 전체가 이 평면의 바깥에 존재하므로 절두체 밖에 있는 것이 확정됩니다.
		if (SignedDistance < 0.0f)
		{
			// 하나의 평면에 대해서라도 완전히 외부에 있다면, 더 검사할 필요 없이 컬링 대상입니다.
			return false;
		}
	}

	// 6개의 모든 평면 검사를 통과했다면, AABB는 절두체와 교차하거나 내부에 있는 것입니다.
	return true;
}
