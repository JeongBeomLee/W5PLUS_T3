#pragma once
#include "pch.h"
#include "AABoundingBoxComponent.h"
#include "Actor.h"
struct FMidPhaseBVHPrimitive
{
	AActor* Actor;
	FBound Bounds; // actor's nounds
};
struct FMidPhaseBVHNode
{
	FBound Bounds; // node's bounds - 자신과 모든 자손 노드에 포함된 프리미티브를 감싸는 거대한 aabb
	FMidPhaseBVHNode* Left = nullptr;
	FMidPhaseBVHNode* Right = nullptr;

	// It's meaningful when this node is leaf
	TArray<FMidPhaseBVHPrimitive> Primitives; // only leaf
	

	FMidPhaseBVHNode() = default;
	bool IsLeafNode() const { return Left == nullptr; }

};
struct FNarrowPhaseBVHPrimitive
{
	int32 TriangleIndex; // in mesh
	FBound Bounds;		 // triangle's bound
};
struct FNarrowPhaseBVHNode
{
	FBound Bounds; // node's bounds - 자신과 모든 자손 노드에 포함된 프리미티브를 감싸는 거대한 aabb
	FNarrowPhaseBVHNode* Left;
	FNarrowPhaseBVHNode* Right;

	// It's meaningful when this node is leaf
	TArray<FNarrowPhaseBVHPrimitive> Primitives;
	
	FNarrowPhaseBVHNode() = default;
	bool IsLeaf() const { return Left == nullptr; }
	
};

class FBVHBuilder
{
public:
	template<typename TNode, typename TPrimitive>
	TNode* Build(const TArray<TPrimitive>& Primitives)
	{
		if (Primitives.IsEmpty())
		{
			return nullptr;
		}
		return RecursiveBuild<TNode, TPrimitive>(Primitives);
	}
	template<typename TNode>
	void Cleanup(TNode* Node)
	{
		if (!Node) return;

		Cleanup(Node->Left);
		Cleanup(Node->Right);
		delete Node;
	}

private:
	template<typename TNode, typename TPrimitive>
	TNode* RecursiveBuild(TArray<TPrimitive> Primitives)
	{
		// 종료 조건: 프리미티브 수가 임계값 이하면 리프노드 생성
		if (Primitives.Num() <= 4)
		{
			TNode* LeafNode = new TNode();
			LeafNode->Primitives = Primitives;
			LeafNode->Bounds = CalculateBounds(Primitives);
			return LeafNode;
		}
		// 중간 노드 생성 및 전체 bound 계산
		TNode* Node = new TNode();
		Node->Bounds = CalculateBounds(Primitives);

		// 가장 긴 축 찾기
		const FVector Extent = Node->Bounds.GetExtent();
		int32 Axis = 0; // 0: X, 1: Y, 2: Z
		if (Extent.Y > Extent.X && Extent.Y > Extent.Z) Axis = 1;
		if (Extent.Z > Extent.Y && Extent.Z > Extent.X) Axis = 2;
		
		// 긴 축 기준으로 프리미티브 정렬
		Primitives.Sort([Axis](const TPrimitive& A, const TPrimitive& B) {
			return A.Bounds.GetCenter()[Axis] < B.Bounds.GetCenter()[Axis];
			});
		
		// Median값 기준으로 두 그룹 분할
		const int32 MidIndex = Primitives.Num() / 2;
		TArray<TPrimitive> LeftPrimitives;
		TArray<TPrimitive> RightPrimitives;

		// 왼쪽 그룹 채우기
		LeftPrimitives.Reserve(MidIndex);
		for (int32 i = 0; i < MidIndex; ++i)
		{
			LeftPrimitives.Add(Primitives[i]);
		}

		// 오른쪽 그룹 채우기
		RightPrimitives.Reserve(Primitives.Num() - MidIndex); 
		for (int32 i = MidIndex; i < Primitives.Num(); ++i)
		{
			RightPrimitives.Add(Primitives[i]);
		}

		// 각 그룹에 대해 재귀적으로 자식 노드 생성
		Node->Left = RecursiveBuild<TNode, TPrimitive>(LeftPrimitives);
		Node->Right = RecursiveBuild<TNode, TPrimitive>(RightPrimitives);

		return Node;
	}
	/**
	* @brief 프리미티브 목록 전체를 감싸는 AABB 계산
	* @param Primitives 프리미티브들의 배열
	*/
	template<typename TPrimitive>
	FBound CalculateBounds(const TArray<TPrimitive>& Primitives)
	{
		if (Primitives.IsEmpty())
		{
			return FBound(FVector(0, 0, 0), FVector(0, 0, 0));
		}
		FBound TotalBound(FVector(FLT_MAX, FLT_MAX, FLT_MAX), FVector(-FLT_MAX, -FLT_MAX, -FLT_MAX));

		for (const auto& Primitive : Primitives)
		{
			TotalBound += Primitive.Bounds;
		}
		return TotalBound;
	}
};
