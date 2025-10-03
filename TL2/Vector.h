#pragma once
#include <cmath>        // ← 추가 (std::sin, std::cos, std::atan2, std::copysign 등)
#include <algorithm>
#include <string>
#include <limits>

#include "UEContainer.h"
#include "UI/GlobalConsole.h"

// 혹시 다른 헤더에서 새어 들어온 매크로 방지
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif
constexpr float KINDA_SMALL_NUMBER = 1e-6f;

// ─────────────────────────────
// Constants & Helpers
// ─────────────────────────────
constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 6.2831853071795864769f;
constexpr float HALF_PI = 1.5707963267948966192f;
constexpr float ToRadian = PI / 180.0f;
constexpr float ToDegree = 180.0f / PI;
inline float DegreeToRadian(float Degree) { return Degree * (PI / 180.0f); }
inline float RadianToDegree(float Radian) { return Radian * (180.0f / PI); }
// FMath 네임스페이스 대체
namespace FMath {
    template<typename T>
    static T Max(T A, T B) { return std::max(A, B); }
    template<typename T>
    static T Min(T A, T B) { return std::min(A, B); }
    template<typename T>
    static T Clamp(T Value, T Min, T Max) {
        return Value < Min ? Min : (Value > Max ? Max : Value);
    }
}
// 각도를 -180 ~ 180 범위로 정규화 (모듈러 연산)
inline float NormalizeAngleDeg(float angleDeg)
{
    // fmod로 -360 ~ 360 범위로 만든 후, -180 ~ 180 범위로 조정
    angleDeg = std::fmod(angleDeg, 360.0f);
    if (angleDeg > 180.0f)
        angleDeg -= 360.0f;
    else if (angleDeg < -180.0f)
        angleDeg += 360.0f;
    return angleDeg;
}

// ─────────────────────────────
// Forward Declarations
// ─────────────────────────────
struct FVector;
struct FVector4;
struct FQuat;
struct FMatrix;
struct FTransform;

// ─────────────────────────────
// FVector (2D Vector)
// ─────────────────────────────

//// Add this global operator== for FVector2D and FVector4 to fix E0349 errors
//inline bool operator==(const FVector2D& A, const FVector2D& B)
//{
//    return std::fabs(A.X - B.X) < KINDA_SMALL_NUMBER &&
//        std::fabs(A.Y - B.Y) < KINDA_SMALL_NUMBER;
//}
//
//inline bool operator!=(const FVector2D& A, const FVector2D& B)
//{
//    return !(A == B);
//}
//
//inline bool operator==(const FVector4& A, const FVector4& B)
//{
//    return std::fabs(A.X - B.X) < KINDA_SMALL_NUMBER &&
//        std::fabs(A.Y - B.Y) < KINDA_SMALL_NUMBER &&
//        std::fabs(A.Z - B.Z) < KINDA_SMALL_NUMBER &&
//        std::fabs(A.W - B.W) < KINDA_SMALL_NUMBER;
//}
//
//inline bool operator!=(const FVector4& A, const FVector4& B)
//{
//    return !(A == B);
//}

struct FVector2D
{
    float X, Y;

    FVector2D(float InX = 0.0f, float InY = 0.0f) : X(InX), Y(InY)
    {
    }

    bool operator==(const FVector2D& V) const
    {
        return std::fabs(X - V.X) < KINDA_SMALL_NUMBER &&
            std::fabs(Y - V.Y) < KINDA_SMALL_NUMBER;
    }
    bool operator!=(const FVector2D& V) const { return !(*this == V); }

    FVector2D operator-(const FVector2D& Other) const
    {
        return FVector2D(X - Other.X, Y - Other.Y);
    }

    FVector2D operator+(const FVector2D& Other) const
    {
        return FVector2D(X + Other.X, Y + Other.Y);
    }

    FVector2D operator*(float Scalar) const
    {
        return FVector2D(X * Scalar, Y * Scalar);
    }

    float Length() const
    {
        return std::sqrt(X * X + Y * Y);
    }

    FVector2D GetNormalized() const
    {
        float Len = Length();
        if (Len > 0.0001f)
            return FVector2D(X / Len, Y / Len);
        return FVector2D(0.0f, 0.0f);
    }
};


// ─────────────────────────────
// FVector (3D Vector)
// ─────────────────────────────
struct FVector
{
    float X, Y, Z;

    FVector(float InX = 0, float InY = 0, float InZ = 0)
        : X(InX), Y(InY), Z(InZ)
    {
    }

    // 기본 연산
    FVector operator+(const FVector& V) const { return FVector(X + V.X, Y + V.Y, Z + V.Z); }
    FVector operator-(const FVector& V) const { return FVector(X - V.X, Y - V.Y, Z - V.Z); }
    FVector operator*(const FVector& V) const { return FVector(X * V.X, Y * V.Y, Z * V.Z); }
    FVector operator*(float S)        const { return FVector(X * S, Y * S, Z * S); }
    FVector operator/(float S)        const { return FVector(X / S, Y / S, Z / S); }
    FVector operator+(float S)        const { return FVector(X + S, Y + S, Z + S); }
    FVector operator-(float S)        const { return FVector(X - S, Y - S, Z - S); }
    FVector operator-()               const { return FVector(-X, -Y, -Z); }

    FVector& operator+=(const FVector& V) { X += V.X; Y += V.Y; Z += V.Z; return *this; }
    FVector& operator-=(const FVector& V) { X -= V.X; Y -= V.Y; Z -= V.Z; return *this; }
    FVector& operator*=(float S) { X *= S; Y *= S; Z *= S; return *this; }
    FVector& operator/=(float S) { X /= S; Y /= S; Z /= S; return *this; }
    FVector& operator+=(float S) { X += S; Y += S; Z += S; return *this; }
    FVector& operator-=(float S) { X -= S; Y -= S; Z -= S; return *this; }

    bool operator==(const FVector& V) const
    {
        return std::fabs(X - V.X) < KINDA_SMALL_NUMBER &&
            std::fabs(Y - V.Y) < KINDA_SMALL_NUMBER &&
            std::fabs(Z - V.Z) < KINDA_SMALL_NUMBER;
    }
    bool operator!=(const FVector& V) const { return !(*this == V); }

    // 인덱스 접근자 (0=X, 1=Y, 2=Z)
    float& operator[](int Index)
    {
        switch (Index)
        {
        case 0: return X;
        case 1: return Y;
        case 2: return Z;
        default: return X; // 안전한 기본값
        }
    }

    const float& operator[](int Index) const
    {
        switch (Index)
        {
        case 0: return X;
        case 1: return Y;
        case 2: return Z;
        default: return X; // 안전한 기본값
        }
    }

    FVector ComponentMin(const FVector& B)
    {
        return FVector(
            (X < B.X) ? X : B.X,
            (Y < B.Y) ? Y : B.Y,
            (Z < B.Z) ? Z : B.Z
        );
    }
    FVector ComponentMax(const FVector& B)
    {
        return FVector(
            (X > B.X) ? X : B.X,
            (Y > B.Y) ? Y : B.Y,
            (Z > B.Z) ? Z : B.Z
        );
    }
    // 크기
    float Size()         const { return std::sqrt(X * X + Y * Y + Z * Z); }
    float SizeSquared()  const { return X * X + Y * Y + Z * Z; }

    // 정규화
    FVector GetNormalized() const
    {
        float S = Size();
        return (S > KINDA_SMALL_NUMBER) ? (*this / S) : FVector(0, 0, 0);
    }
    void Normalize()
    {
        float S = Size();
        if (S > KINDA_SMALL_NUMBER) { X /= S; Y /= S; Z /= S; }
    }
    FVector GetSafeNormal() const { return GetNormalized(); }

    // 내적/외적
    float Dot(const FVector& V) const { return X * V.X + Y * V.Y + Z * V.Z; }
    static float   Dot(const FVector& A, const FVector& B) { return A.X * B.X + A.Y * B.Y + A.Z * B.Z; }
    static FVector Cross(const FVector& A, const FVector& B)
    {
        return FVector(
            A.Y * B.Z - A.Z * B.Y,
            A.Z * B.X - A.X * B.Z,
            A.X * B.Y - A.Y * B.X
        );
    }


    // 보조 유틸
    static FVector Lerp(const FVector& A, const FVector& B, float T)
    {
        return A + (B - A) * T;
    }
    static float Distance(const FVector& A, const FVector& B)
    {
        return (B - A).Size();
    }
    static float AngleBetween(const FVector& A, const FVector& B) // radians
    {
        float D = Dot(A, B) / (std::sqrt(A.SizeSquared() * B.SizeSquared()) + KINDA_SMALL_NUMBER);
        D = std::max(-1.0f, std::min(1.0f, D));
        return std::acos(D);
    }
    static FVector Project(const FVector& A, const FVector& OnNormal)
    {
        FVector N = OnNormal.GetNormalized();
        return N * Dot(A, N);
    }
    static FVector Reflect(const FVector& V, const FVector& Normal) // Normal normalized
    {
        return V - Normal * (2.0f * Dot(V, Normal));
    }
    static FVector Clamp(const FVector& V, float minLen, float maxLen)
    {
        float Length = V.Size();
        if (Length < KINDA_SMALL_NUMBER) return FVector(0, 0, 0);
        float Value = std::max(minLen, std::min(maxLen, Length));
        return V * (Value / Length);
    }

    static FVector One()
    {
        return FVector(1.f, 1.f, 1.f);
    }
    const static FVector Forward;
    const static FVector Right;
    const static FVector Up;
    const static FVector UnitX;
    const static FVector UnitY;
    const static FVector UnitZ;
    const static FVector One;
    const static FVector Zero;

    void Log()
    {
        char debugMsg[64];
        sprintf_s(debugMsg, "Vector3(%f, %f, %f)", X, Y, Z);
        UE_LOG(debugMsg);
    }
};

// ─────────────────────────────
// FVector4 (4D Vector)
// ─────────────────────────────
struct FVector4
{
    float X, Y, Z, W;
    FVector4(float InX = 0, float InY = 0, float InZ = 0, float InW = 0)
        : X(InX), Y(InY), Z(InZ), W(InW)
    {
    }

    FVector4 ComponentMin(const FVector4& B)
    {
        return FVector4(
            (X < B.X) ? X : B.X,
            (Y < B.Y) ? Y : B.Y,
            (Z < B.Z) ? Z : B.Z,
            (W < B.W) ? W : B.W
        );
    }
    FVector4 ComponentMax(const FVector4& B)
    {
        return FVector4(
            (X > B.X) ? X : B.X,
            (Y > B.Y) ? Y : B.Y,
            (Z > B.Z) ? Z : B.Z,
            (W > B.W) ? W : B.W
        );
    }
    void Log()
    {
        char debugMsg[64];
        sprintf_s(debugMsg, "Vector4(%f, %f, %f, %f)", X, Y, Z, W);
        UE_LOG(debugMsg);
    }
};

// ─────────────────────────────
// FQuat (Quaternion)
// ─────────────────────────────
struct FQuat
{
public:

    float X, Y, Z, W;

    static FQuat Identity;
public:

    FQuat() :X(0), Y(0), Z(0), W(0) {}
    FQuat(float x, float y, float z, float w) :X(x), Y(y), Z(z), W(w) {}
    FQuat(const FVector& normal, const float degree);
    FVector RotateVector(const FVector& v)const;
    FQuat RotateAxis(const FVector& axis, const float degree);
    FVector ToEulerDegree();
    FVector GetForward() const;
    FVector GetUp() const;
    FVector GetRight() const;

    void Normalize();
    const FMatrix ToMatrix() const;
    static FQuat MakeFromEuler(const FVector& eulerDegree);
    FQuat operator* (const FQuat& rhs) const;

};

// ─────────────────────────────
// FMatrix (4x4 Matrix)
// (Row-major, Translation in M[row][3])
// ─────────────────────────────
struct alignas(16) FMatrix
{
    union // 같은 데이터를 다양한 방법으로 참조하기 위한 키워드
    {
        float M[4][4]{};
        float FlatM[16]; // 1차원 배열로 접근
        FVector4 Rows[4];  // FVector4 행으로 접근
    };

    FMatrix() = default;

    constexpr FMatrix(float M00, float M01, float M02, float M03,
        float M10, float M11, float M12, float M13,
        float M20, float M21, float M22, float M23,
        float M30, float M31, float M32, float M33) noexcept
    {
        M[0][0] = M00; M[0][1] = M01; M[0][2] = M02; M[0][3] = M03;
        M[1][0] = M10; M[1][1] = M11; M[1][2] = M12; M[1][3] = M13;
        M[2][0] = M20; M[2][1] = M21; M[2][2] = M22; M[2][3] = M23;
        M[3][0] = M30; M[3][1] = M31; M[3][2] = M32; M[3][3] = M33;
    }
    constexpr FMatrix(const FVector4& Row0, const FVector4& Row1, const FVector4& Row2, const FVector4& Row3)
    {
        Rows[0] = Row0;
        Rows[1] = Row1;
        Rows[2] = Row2;
        Rows[3] = Row3;
    }

    static FMatrix Identity()
    {
        return FMatrix(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }
    // 행렬 == 행렬
    bool operator==(const FMatrix& B) const noexcept
    {
        const FMatrix& A = *this;
        for (uint8 i = 0; i < 4; ++i)
        {
            for (uint8 j = 0; j < 4; ++j)
            {
                if (A.M[i][j] != B.M[i][j])
                    return false;
            }
        }
        return true;
    }

    // 행렬 != 행렬
    bool operator!=(const FMatrix& B) const noexcept
    {
        return !(*this == B);
    }
    // 행렬 * 행렬
    FMatrix operator*(const FMatrix& B) const
    {
        const FMatrix& A = *this;
        FMatrix C;
        for (uint8 I = 0; I < 4; ++I)
        {
            for (uint8 J = 0; J < 4; ++J)
            {
                C.M[I][J] = A.M[I][0] * B.M[0][J]
                    + A.M[I][1] * B.M[1][J]
                    + A.M[I][2] * B.M[2][J]
                    + A.M[I][3] * B.M[3][J];
            }
        }
        return C;
    }

    // 전치
    FMatrix Transpose() const
    {
        FMatrix T;
        for (uint8 i = 0; i < 4; ++i)
        {
            for (uint8 j = 0; j < 4; ++j)
            {
                T.M[i][j] = M[j][i];
            }
        }
        return T;
    }
    FVector GetForward() const
    {
        return FVector(M[0][0], M[0][1], M[0][2]).GetNormalized();
    }
    FVector GetRight() const
    {
        return FVector(M[1][0], M[1][1], M[1][2]).GetNormalized();
    }
    FVector GetUp() const
    {
        return FVector(M[2][0], M[2][1], M[2][2]).GetNormalized();
    }
    // Affine 역행렬 (마지막 행 = [0,0,0,1] 가정)
    FMatrix InverseAffine() const
    {
        // 상단 3x3 역행렬
        float A00 = M[0][0], A01 = M[0][1], A02 = M[0][2];
        float A10 = M[1][0], A11 = M[1][1], A12 = M[1][2];
        float A20 = M[2][0], A21 = M[2][1], A22 = M[2][2];

        float Det = A00 * (A11 * A22 - A12 * A21) - A01 * (A10 * A22 - A12 * A20) + A02 * (A10 * A21 - A11 * A20);
        if (std::fabs(Det) < KINDA_SMALL_NUMBER) return Identity();
        float InvDet = 1.0f / Det;

        // Rinv = R^{-1}
        FMatrix InvMat = Identity();
        InvMat.M[0][0] = (A11 * A22 - A12 * A21) * InvDet;
        InvMat.M[0][1] = -(A01 * A22 - A02 * A21) * InvDet;
        InvMat.M[0][2] = (A01 * A12 - A02 * A11) * InvDet;

        InvMat.M[1][0] = -(A10 * A22 - A12 * A20) * InvDet;
        InvMat.M[1][1] = (A00 * A22 - A02 * A20) * InvDet;
        InvMat.M[1][2] = -(A00 * A12 - A02 * A10) * InvDet;

        InvMat.M[2][0] = (A10 * A21 - A11 * A20) * InvDet;
        InvMat.M[2][1] = -(A00 * A21 - A01 * A20) * InvDet;
        InvMat.M[2][2] = (A00 * A11 - A01 * A10) * InvDet;

        // t: last row
        const FVector T(M[3][0], M[3][1], M[3][2]);

        // invT = -t * Rinv  (t treated as row-vector)
        FVector invT(
            -(T.X * InvMat.M[0][0] + T.Y * InvMat.M[1][0] + T.Z * InvMat.M[2][0]),
            -(T.X * InvMat.M[0][1] + T.Y * InvMat.M[1][1] + T.Z * InvMat.M[2][1]),
            -(T.X * InvMat.M[0][2] + T.Y * InvMat.M[1][2] + T.Z * InvMat.M[2][2])
        );

        FMatrix Out = Identity();
        // place Rinv in upper-left
        Out.M[0][0] = InvMat.M[0][0]; Out.M[0][1] = InvMat.M[0][1]; Out.M[0][2] = InvMat.M[0][2];
        Out.M[1][0] = InvMat.M[1][0]; Out.M[1][1] = InvMat.M[1][1]; Out.M[1][2] = InvMat.M[1][2];
        Out.M[2][0] = InvMat.M[2][0]; Out.M[2][1] = InvMat.M[2][1]; Out.M[2][2] = InvMat.M[2][2];
        // translation in last row
        Out.M[3][0] = invT.X; Out.M[3][1] = invT.Y; Out.M[3][2] = invT.Z; Out.M[3][3] = 1.0f;
        return Out;
    }

    static FMatrix FromTRS(const FVector& T, const FQuat& R, const FVector& S)
    {
        FMatrix rot = R.ToMatrix();
        // scale 적용
        rot.M[0][0] *= S.X; rot.M[0][1] *= S.X; rot.M[0][2] *= S.X;
        rot.M[1][0] *= S.Y; rot.M[1][1] *= S.Y; rot.M[1][2] *= S.Y;
        rot.M[2][0] *= S.Z; rot.M[2][1] *= S.Z; rot.M[2][2] *= S.Z;
        // translation
        rot.M[3][0] = T.X;
        rot.M[3][1] = T.Y;
        rot.M[3][2] = T.Z;
        rot.M[3][3] = 1;
        return rot;
    }
    static FMatrix Translation(const FVector& T)
    {
        FMatrix TMat = FMatrix::Identity();
        TMat.M[3][0] = T.X;
        TMat.M[3][1] = T.Y;
        TMat.M[3][2] = T.Z;
        return TMat;
    }
    // View/Proj (L H)
    static FMatrix LookAtLH(const FVector& Eye, const FVector& At, const FVector& Up);
    static FMatrix PerspectiveFovLH(float FovY, float Aspect, float Zn, float Zf);
    static FMatrix OrthoLH(float Width, float Height, float Zn, float Zf);

    void Log() const
    {
        char debugMsg[256];
        sprintf_s(debugMsg, "Matrix\n%f, %f, %f, %f \n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f",
            FlatM[0], FlatM[1], FlatM[2], FlatM[3],
            FlatM[4], FlatM[5], FlatM[6], FlatM[7],
            FlatM[8], FlatM[9], FlatM[10], FlatM[11],
            FlatM[12], FlatM[13], FlatM[14], FlatM[15]);
        UE_LOG(debugMsg);
    }

    static const FMatrix ViewAxis;
};

inline FVector operator*(const FVector& V, const FMatrix& S)
{
    FVector Result;
    Result.X = V.X * S.M[0][0] + V.Y * S.M[1][0] + V.Z * S.M[2][0] + S.M[3][0];
    Result.Y = V.X * S.M[0][1] + V.Y * S.M[1][1] + V.Z * S.M[2][1] + S.M[3][1];
    Result.Z = V.X * S.M[0][2] + V.Y * S.M[1][2] + V.Z * S.M[2][2] + S.M[3][2];
    return Result;
}

// ─────────────────────────────
// 전역 연산자들
// ─────────────────────────────

// FVector4 * FMatrix (row-vector: v' = v * M)
inline FVector4 operator*(const FVector4& V, const FMatrix& M)
{
    return FVector4(
        V.X * M.M[0][0] + V.Y * M.M[1][0] + V.Z * M.M[2][0] + V.W * M.M[3][0],
        V.X * M.M[0][1] + V.Y * M.M[1][1] + V.Z * M.M[2][1] + V.W * M.M[3][1],
        V.X * M.M[0][2] + V.Y * M.M[1][2] + V.Z * M.M[2][2] + V.W * M.M[3][2],
        V.X * M.M[0][3] + V.Y * M.M[1][3] + V.Z * M.M[2][3] + V.W * M.M[3][3]
    );
}

// ─────────────────────────────
// FTransform (position/rotation/scale)
// ─────────────────────────────
struct FTransform
{
    FVector Translation;
    FQuat   Rotation;
    FVector Scale3D;

    FTransform() : Rotation(0, 0, 0, 1), Translation(0, 0, 0), Scale3D(1, 1, 1) {}
    FTransform(const FVector& T, const FQuat& R, const FVector& S) : Rotation(R), Translation(T), Scale3D(S) {}

    FMatrix ToMatrixWithScaleLocalXYZ() const;
    // 합성 (this * Other)
    FTransform operator*(const FTransform& Other) const;

    // 역변환
    FTransform Inverse() const;

    FMatrix GetWorldMatrix() const;


    //static FTransform Lerp(const FTransform& A, const FTransform& B, float T)
    //{
    //    FVector  TPosition = FVector::Lerp(A.Translation, B.Translation, T);
    //    FVector  TScale = FVector::Lerp(A.Scale3D, B.Scale3D, T);
    //    FQuat    TRotation = FQuat::Slerp(A.Rotation, B.Rotation, T);
    //    return FTransform(TPosition, TRotation, TScale);
    //}
};

// ─────────────────────────────
// Inline 구현부
// ─────────────────────────────

// Row-major + 행벡터(p' = p * M), Left-Handed: forward = +Z
inline FMatrix FMatrix::LookAtLH(const FVector& Eye, const FVector& At, const FVector& Up)
{
    // 1) forward(+Z)
    FVector ZAxis = (At - Eye);
    const float LenZ2 = ZAxis.X * ZAxis.X + ZAxis.Y * ZAxis.Y + ZAxis.Z * ZAxis.Z;
    if (LenZ2 < 1e-12f) // Eye == At
        return FMatrix::Identity();
    ZAxis = ZAxis / sqrt(LenZ2);

    // 2) right = Up × forward (LH에서 이 순서가 오른손쪽을 만듦)
    FVector XAxis = FVector::Cross(Up, ZAxis);
    float LenX2 = XAxis.X * XAxis.X + XAxis.Y * XAxis.Y + XAxis.Z * XAxis.Z;
    if (LenX2 < 1e-12f)
    { // up이 forward와 평행/반평행이면 임의 보정 벡터 사용
        const FVector Temp(0.0f, 1.0f, 0.0f);
        XAxis = FVector::Cross(Temp, ZAxis);
        LenX2 = XAxis.X * XAxis.X + XAxis.Y * XAxis.Y + XAxis.Z * XAxis.Z;
        if (LenX2 < 1e-12f)
        {
            const FVector Tmp2(1.0f, 0.0f, 0.0f);
            XAxis = FVector::Cross(Tmp2, ZAxis);
            LenX2 = XAxis.X * XAxis.X + XAxis.Y * XAxis.Y + XAxis.Z * XAxis.Z;
            if (LenX2 < 1e-12f) return FMatrix::Identity();
        }
    }
    XAxis = XAxis / sqrt(LenX2);

    // 3) Up = forward × right  (정규직교 보정)
    FVector YAxis = FVector::Cross(ZAxis, XAxis);
    const float LenY2 = YAxis.X * YAxis.X + YAxis.Y * YAxis.Y + YAxis.Z * YAxis.Z;
    if (LenY2 < 1e-12f) return FMatrix::Identity();
    YAxis = YAxis / sqrt(LenY2);

    // 4) 조립 (기저 벡터를 행에, 평행이동을 마지막 행에)
    FMatrix View = FMatrix::Identity();
    View.M[0][0] = XAxis.X; View.M[0][1] = XAxis.Y; View.M[0][2] = XAxis.Z; View.M[0][3] = 0.0f;
    View.M[1][0] = YAxis.X; View.M[1][1] = YAxis.Y; View.M[1][2] = YAxis.Z; View.M[1][3] = 0.0f;
    View.M[2][0] = ZAxis.X; View.M[2][1] = ZAxis.Y; View.M[2][2] = ZAxis.Z; View.M[2][3] = 0.0f;

    // 마지막 행 = -Eye * R (행벡터 규약)
    View.M[3][0] = -(Eye.X * View.M[0][0] + Eye.Y * View.M[0][1] + Eye.Z * View.M[0][2]);
    View.M[3][1] = -(Eye.X * View.M[1][0] + Eye.Y * View.M[1][1] + Eye.Z * View.M[1][2]);
    View.M[3][2] = -(Eye.X * View.M[2][0] + Eye.Y * View.M[2][1] + Eye.Z * View.M[2][2]);
    View.M[3][3] = 1.0f;

    return View;
}


inline FMatrix FMatrix::PerspectiveFovLH(float FovY, float Aspect, float Zn, float Zf)
{
    float YScale = 1.0f / std::tan(FovY * 0.5f);
    float XScale = YScale / Aspect;

    FMatrix proj{};
    proj.M[0][0] = XScale;
    proj.M[1][1] = YScale;
    proj.M[2][2] = Zf / (Zf - Zn);
    proj.M[2][3] = 1.0f;
    proj.M[3][2] = (-Zn * Zf) / (Zf - Zn);
    proj.M[3][3] = 0.0f;
    return proj;
}

inline FMatrix FMatrix::OrthoLH(float Width, float Height, float Zn, float Zf)
{
    // 기본 방어: 0 또는 역Z 방지
    const float W = (Width != 0.0f) ? Width : 1e-6f;
    const float H = (Height != 0.0f) ? Height : 1e-6f;
    const float DZ = (Zf - Zn != 0.0f) ? (Zf - Zn) : 1e-6f;

    FMatrix m = FMatrix::Identity();
    m.M[0][0] = 2.0f / W;
    m.M[1][1] = 2.0f / H;
    m.M[2][2] = 1.0f / DZ;
    m.M[3][2] = -Zn / DZ;   // 행벡터 규약: 마지막 행에 배치
    // 나머지는 Identity()로 이미 [0,0,0,1]
    return m;
}

inline FMatrix MakeRotationRowMajorFromQuat(const FQuat& Q)
{
    // 비정규 안전화
    const float N = Q.X * Q.X + Q.Y * Q.Y + Q.Z * Q.Z + Q.W * Q.W;
    if (N <= 1e-8f) return FMatrix::Identity();
    const float S = 2.0f / N;

    const float XX = Q.X * Q.X * S, YY = Q.Y * Q.Y * S, ZZ = Q.Z * Q.Z * S;
    const float XY = Q.X * Q.Y * S, XZ = Q.X * Q.Z * S, YZ = Q.Y * Q.Z * S;
    const float WX = Q.W * Q.X * S, WY = Q.W * Q.Y * S, WZ = Q.W * Q.Z * S;

    FMatrix M = FMatrix::Identity();
    // row-major + 행벡터용 회전 블록
    M.M[0][0] = 1.0f - (YY + ZZ); M.M[0][1] = XY + WZ;          M.M[0][2] = XZ - WY;            M.M[0][3] = 0.0f;
    M.M[1][0] = XY - WZ;          M.M[1][1] = 1.0f - (XX + ZZ); M.M[1][2] = YZ + WX;            M.M[1][3] = 0.0f;
    M.M[2][0] = XZ + WY;          M.M[2][1] = YZ - WX;          M.M[2][2] = 1.0f - (XX + YY);   M.M[2][3] = 0.0f;
    // 마지막 행은 호출부에서 채움(평행이동 등)
    return M;
}

// 최종: S * R(qXYZ) * T  (row-major + 행벡터 규약)
// row-major + 행벡터(p' = p * M) 규약
inline FMatrix FTransform::ToMatrixWithScaleLocalXYZ() const
{
    FMatrix YUpToZUp =
    {
         0,  1,  0, 0 ,
         0,  0,  1, 0 ,
         1, 0,  0, 0 ,
         0,  0,  0, 1
    };
    // Rotation(FQuat)은 이미 로컬 XYZ 순서로 만들어져 있다고 가정
    FMatrix R = MakeRotationRowMajorFromQuat(Rotation);

    // 행별 스케일(S * R): 각 "행"에 스케일 적용
    R.M[0][0] *= Scale3D.X; R.M[0][1] *= Scale3D.X; R.M[0][2] *= Scale3D.X;
    R.M[1][0] *= Scale3D.Y; R.M[1][1] *= Scale3D.Y; R.M[1][2] *= Scale3D.Y;
    R.M[2][0] *= Scale3D.Z; R.M[2][1] *= Scale3D.Z; R.M[2][2] *= Scale3D.Z;

    // 동차좌표 마무리 + Translation(last row)
    R.M[0][3] = 0.0f; R.M[1][3] = 0.0f; R.M[2][3] = 0.0f;
    R.M[3][0] = Translation.X;
    R.M[3][1] = Translation.Y;
    R.M[3][2] = Translation.Z;
    R.M[3][3] = 1.0f;

    return YUpToZUp * R; // 결과 = S * R(q) * T
    //return R; // 결과 = S * R(q) * T
}


// FTransform 합성 (this * Other)
inline FTransform FTransform::operator*(const FTransform& Other) const
{
    FTransform Result;

    // 회전 결합
    Result.Rotation = Other.Rotation * Rotation;
    Result.Rotation.Normalize();

    // 스케일 결합 (component-wise)
    Result.Scale3D = FVector(
        Scale3D.X * Other.Scale3D.X,
        Scale3D.Y * Other.Scale3D.Y,
        Scale3D.Z * Other.Scale3D.Z
    );

    // 위치 결합: R*(S*Other.T) + T
    FVector Scaled(Other.Translation.X * Scale3D.X,
        Other.Translation.Y * Scale3D.Y,
        Other.Translation.Z * Scale3D.Z);
    FVector Rotated = Rotation.RotateVector(Scaled);
    Result.Translation = Translation + Rotated;

    return Result;
}



// FTransform 역변환
inline FTransform FTransform::Inverse() const
{
    // InvScale
    FVector InvScale(
        (std::fabs(Scale3D.X) > KINDA_SMALL_NUMBER) ? 1.0f / Scale3D.X : 0.0f,
        (std::fabs(Scale3D.Y) > KINDA_SMALL_NUMBER) ? 1.0f / Scale3D.Y : 0.0f,
        (std::fabs(Scale3D.Z) > KINDA_SMALL_NUMBER) ? 1.0f / Scale3D.Z : 0.0f
    );

    // InvRot = conjugate (단위 가정)
    FQuat InvRot(-Rotation.X, -Rotation.Y, -Rotation.Z, Rotation.W);

    // InvTrans = -(InvRot * (InvScale * T))
    FVector Scaled(Translation.X * InvScale.X,
        Translation.Y * InvScale.Y,
        Translation.Z * InvScale.Z);
    FVector Rotated = InvRot.RotateVector(Scaled);
    FVector InvTrans(-Rotated.X, -Rotated.Y, -Rotated.Z);

    FTransform Out;
    Out.Rotation = InvRot;
    Out.Scale3D = InvScale;
    Out.Translation = InvTrans;
    return Out;
}

inline FMatrix FTransform::GetWorldMatrix() const
{
    return FMatrix::FromTRS(Translation, Rotation, Scale3D);
}



struct FPlane
{
    FVector Normal;
    float D;
    FPlane(const FVector& InNormal, const FVector& Pos)
    {
        Normal = InNormal.GetNormalized();
        D = -Normal.Dot(Pos);
    }

    FVector IntersectRay(const FVector& RayDir, const FVector& RayOrigin)
    {
        //HitPos = P, PlaneNormal = N, PlaneD = D, 
        //RayDir = V, RayOrigin = O
        //N * P + D = 0
        //P = t * V + O
        //t를 구하면 됨
        //t(N * V) + N * O + D = 0
        //t = -(N * O + D) / (N * V)
        //평면의 노말과 레이의 방향이 90도면 구할 수 없음
        float NdotV = Normal.Dot(RayDir);
        if (abs(NdotV) < KINDA_SMALL_NUMBER)
        {
            return FVector::Zero;
        }
        float NdotO = Normal.Dot(RayOrigin);
        float T = -(NdotO + D) / NdotV;
        FVector HitPos = RayOrigin + RayDir * T;
        return HitPos;
    }
};