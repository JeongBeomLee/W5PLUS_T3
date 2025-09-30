#pragma once

//#include "VertexSimple.h"
#include "pch.h"
//FVertexSimple GTriangleVertices[] =
//{
//    {  0.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f }, // Top (Red)
//    {  1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-Right (Green)
//    { -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f }  // Bottom-Left (Blue)
//};
struct FTriangle
{
    FVector V0, V1, V2; // 삼각형 월드 공간 정점

    FTriangle() = default;

    FTriangle(const FVector& InV0, const FVector& InV1, const FVector& InV2) :
        V0(InV0), V1(InV1), V2(InV2){ }

    /**
    * @brief 해당 삼각형을 꼭 맞게 감싸는 AABB 계산
    * @return FBound 이 삼각형 경계 상자
    */
    FBound GetBounds() 
    {
        FVector Min = V0.ComponentMin(V1).ComponentMin(V2);
        FVector Max = V0.ComponentMax(V1).ComponentMax(V2);
        return FBound(Min, Max);
    }

    ///**
    //* @brief Ray - triangle
    //* @param Ray 검사할 광선
    //* @param OutT 교차점까지 거리 저장
    //* @return bool 충돌여부
    //*/
    //bool Intersect(const FRay& Ray, float& OutT) const
    //{
    //    const float Epsilon = 1e-6f;

    //    // 삼각형의 두변
    //    FVector Edge1 = V1 - V0;
    //    FVector Edge2 = V2 - V0;

    //    // 광선 방향과 Edge2 외적
    //    FVector PVec = FVector::Cross(Ray.Direction, Edge2);

    //    // 행렬식(determinant) 계산. 0에 가까우면 광선이 삼각형 평면과 평행하다는 의미
    //    float Det = Edge1.Dot(PVec);
    //    if (Det > -Epsilon && Det < Epsilon)
    //    {
    //        return false; // 평행하면 충돌하지 않음
    //    }

    //    float InvDet = 1.0f / Det;

    //    // 광선 원점에서 삼각형의 첫 정점(V0)까지의 벡터
    //    FVector TVec = Ray.Origin - V0;

    //    // 첫 번째 무게중심 좌표(u) 계산 및 범위 확인
    //    float u = TVec.Dot(PVec) * InvDet;
    //    if (u < 0.0f || u > 1.0f)
    //    {
    //        return false; // u가 [0, 1] 범위를 벗어나면 교차점은 삼각형 외부에 있음
    //    }

    //    FVector QVec = FVector::Cross(TVec, Edge1); 

    //    // 두 번째 무게중심 좌표(v) 계산 및 범위 확인
    //    float v = Ray.Direction.Dot(QVec) * InvDet;
    //    if (v < 0.0f || u + v > 1.0f)
    //    {
    //        return false; // v가 음수이거나 u+v가 1을 넘으면 교차점은 삼각형 외부에 있음
    //    }

    //    // 모든 조건을 통과했다면, 교차점까지의 거리(t) 계산
    //    OutT = Edge2.Dot(QVec) * InvDet;

    //    // 교차점이 광선의 시작점보다 앞에 있는 경우에만 유효한 충돌로 인정
    //    return OutT > Epsilon;
    //}

};