#pragma once

class FBound;

// 절두체를 구성하는 6개의 평면
struct FPlane
{
	FVector Normal;
	float   Distance;
};

class FFrustum
{
public:
	// View-Projection 행렬로부터 6개의 평면을 추출하여 절두체를 업데이트합니다.
	void Update(const FMatrix& InViewProjectionMatrix);

	// AABB가 절두체에 의해 잠재적으로 보이는 상태인지 검사합니다.
	//  AABB가 보이거나 교차하면 true, 완전히 보이지 않아 컬링 대상이면 false를 반환합니다.
	bool IsVisible(FBound& InBox) const;

private:
	FPlane Planes[6];
};