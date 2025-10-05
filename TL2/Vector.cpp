#include "pch.h"
#include "Vector.h"

const FVector FVector::Forward = FVector(1, 0, 0);
const FVector FVector::Right = FVector(0, 1, 0);
const FVector FVector::Up = FVector(0, 0, 1);
const FVector FVector::UnitX = FVector(1, 0, 0);
const FVector FVector::UnitY = FVector(0, 1, 0);
const FVector FVector::UnitZ = FVector(0, 0, 1);
const FVector FVector::One = FVector(1, 1, 1);
const FVector FVector::Zero = FVector(0, 0, 0);

const FMatrix FMatrix::ViewAxis = FMatrix
(
	0, 0, 1, 0,
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 1
);

FQuat FQuat::Identity = FQuat(0, 0, 0, 1);


/// <summary>
///	벡터v를 회전 축 normal로 degree만큼 회전시켜주는 쿼터니온
/// </summary>
/// <param name="normal"></param>
/// <param name="degree"></param>
FQuat::FQuat(const FVector& normal, const float degree)
{
	float theta = degree * ToRadian * 0.5f;
	float cos = cosf(theta);
	float sin = sinf(theta);

	W = cos;
	FVector xyz = normal * sin;
	X = xyz.X;
	Y = xyz.Y;
	Z = xyz.Z;
}

//X = pith
//Y = yaw
//Z = roll
FQuat FQuat::MakeFromEuler(const FVector& eulerDegree)
{
	FVector eulerHalfRad = eulerDegree * ToRadian * 0.5f;
	float cp = cosf(eulerHalfRad.X);
	float cy = cosf(eulerHalfRad.Y);
	float cr = cosf(eulerHalfRad.Z);
	float sp = sinf(eulerHalfRad.X);
	float sy = sinf(eulerHalfRad.Y);
	float sr = sinf(eulerHalfRad.Z);

	FQuat quat;
	quat.W = sy * sp * sr + cy * cp * cr;
	quat.X = sy * sr * cp + sp * cy * cr;
	quat.Y = sy * cp * cr - sp * sr * cy;
	quat.Z = -sy * sp * cr + sr * cy * cp;
	return quat;
}

/// <summary>
/// 현재 쿼터니온에 축과 회전각도에 대한 정보가 저장되어 있음
/// </summary>
/// <param name="v"></param>
/// <returns></returns>
FVector FQuat::RotateVector(const FVector& v)const
{
	//이득우 게임수학 571p
	//각도 = 세타 * 0.5f
	FVector q(X, Y, Z); //(cos, sin * n) 에서 (sin * n)에 해당하는 값
	FVector t = FVector::Cross(q, v) * 2.0f;
	FVector result = v + (t * W) + FVector::Cross(q, t);  //W = cos
	return result;
}
FQuat FQuat::RotateAxis(const FVector& axis, const float degree)
{
	float w = cosf(degree * ToRadian * 0.5f);
	FVector r = axis * sinf(degree * ToRadian * 0.5f);
	FVector v = FVector(X, Y, Z);
	FVector t = FVector::Cross(r, v) * 2;

	FVector resultVt3 = v + t * w + FVector::Cross(r, t);
	return FQuat(resultVt3, 0.0f);
}
FVector FQuat::ToEulerDegree()
{
	Normalize();
	FVector result;
	float sinrCosp = 2 * (W * Z + X * Y);
	float cosrCosp = 1 - 2 * (Z * Z + X * X);
	result.Z = atan2f(sinrCosp, cosrCosp) * ToDegree;

	float pitchTest = W * X - Y * Z;
	float asinThreshold = 0.4999995f;
	float sinp = 2 * pitchTest;

	if (pitchTest < -asinThreshold)
	{
		result.X = -90.0f;
	}
	else if (pitchTest > asinThreshold)
	{
		result.X = 90.0f;
	}
	else
	{
		result.X = asinf(sinp) * ToDegree;
	}

	float sinyCosp = 2 * (W * Y + X * Z);
	float cosyCosp = 1.0f - 2 * (X * X + Y * Y);
	result.Y = atan2f(sinyCosp, cosyCosp) * ToDegree;
	return result;
}
const FMatrix FQuat::ToMatrix()const
{
	FVector4 localAxisX = FVector4(1 - 2 * (Y * Y + Z * Z), 2 * (X * Y + Z * W), 2 * (X * Z - Y * W), 0);
	FVector4 localAxisY = FVector4(2 * (X * Y - Z * W), 1 - 2 * (X * X + Z * Z), 2 * (Y * Z + X * W), 0);
	FVector4 localAxisZ = FVector4(2 * (X * Z + Y * W), 2 * (Y * Z - X * W), 1 - 2 * (X * X + Y * Y), 0);
	FMatrix result = FMatrix(localAxisX, localAxisY, localAxisZ, FVector4(0, 0, 0, 1));
	return result;
}

FVector FQuat::GetForward() const
{
	return	FVector(1 - 2 * (Y * Y + Z * Z), 2 * (X * Y + Z * W), 2 * (X * Z - Y * W));
}
FVector FQuat::GetRight() const
{
	return	FVector(2 * (X * Y - Z * W), 1 - 2 * (X * X + Z * Z), 2 * (Y * Z + X * W));
}
FVector FQuat::GetUp() const
{
	return	FVector(2 * (X * Z + Y * W), 2 * (Y * Z - X * W), 1 - 2 * (X * X + Y * Y));
}
FVector FQuat::GetAxis() const
{
	float radian = acosf(W) * 2;
	if (radian < KINDA_SMALL_NUMBER)
	{
		return FVector::One;
	}
	return FVector(X, Y, Z) / sin(radian * 0.5f);
}
float FQuat::GetAngle() const
{
	return acosf(W) * 2 * ToDegree;
}
FQuat FQuat::GetConjugate() const
{
	return	FQuat(FVector(-X, -Y, -Z), W);
}
void FQuat::Normalize()
{
	float len = sqrtf(X * X + Y * Y + Z * Z + W * W);
	if (len > 1e-6f) // 0으로 나누는 걸 방지
	{
		float rcpLen = 1 / len;
		X *= rcpLen;
		Y *= rcpLen;
		Z *= rcpLen;
		W *= rcpLen;
	}
	else
	{
		// 길이가 거의 0이면 단위 쿼터니언으로 초기화
		X = Y = Z = 0.0f;
		W = 1.0f;
	}
}

FQuat FQuat::operator*	(const FQuat& quat) const
{
	FQuat result;
	FVector v1(X, Y, Z);
	FVector v2(quat.X, quat.Y, quat.Z);
	result.W = W * quat.W - v1.Dot(v2);
	FVector v = v1 * quat.W + v2 * W + FVector::Cross(v1, v2);
	result.X = v.X;
	result.Y = v.Y;
	result.Z = v.Z;
	result.Normalize();
	return result;
}
