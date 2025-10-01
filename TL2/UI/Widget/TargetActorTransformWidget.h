#pragma once
#include "Widget.h"
#include "../../Vector.h"
#include "../../Enums.h"

class UUIManager;
class UWorld;
class AActor;
class AGizmoActor;
class USceneComponent;

class UTargetActorTransformWidget
	: public UWidget
{
public:
	DECLARE_CLASS(UTargetActorTransformWidget, UWidget)

	void Initialize() override;
	void Update() override;
	void RenderWidget() override;

	// Special Member Function
	UTargetActorTransformWidget();
	~UTargetActorTransformWidget() override;

	// 선택된 액터가 외부에서 삭제되었을 때 호출되어 내부 상태를 정리
	void OnSelectedActorCleared();

private:
	UUIManager* UIManager = nullptr;
	AActor* SelectedActor = nullptr;
	USceneComponent* SelectedComponent = nullptr;
	FString CachedActorName; // 액터 이름 캐시 (안전한 출력을 위해)

	// 기즈모 설정
	EGizmoSpace CurrentGizmoSpace = EGizmoSpace::World;
	AGizmoActor* GizmoActor = nullptr;

	// 월드 정보 (옵션)
	uint32 WorldActorCount = 0;

};
