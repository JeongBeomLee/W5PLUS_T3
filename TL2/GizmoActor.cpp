#include "pch.h"
#include "GizmoActor.h"
#include "GizmoArrowComponent.h"
#include "GizmoScaleComponent.h"
#include "GizmoRotateComponent.h"
#include "CameraActor.h"
#include "SelectionManager.h"
#include "InputManager.h"
#include "UI/UIManager.h"
#include"FViewport.h"
#include "Picking.h"

AGizmoActor::AGizmoActor()
{
	Name = "Gizmo Actor";

	//======= Arrow Component 생성 =======
	ArrowX = NewObject<UGizmoArrowComponent>();
	ArrowY = NewObject<UGizmoArrowComponent>();
	ArrowZ = NewObject<UGizmoArrowComponent>();

	ArrowX->SetDirection(FVector(1.0f, 0.0f, 0.0f));//빨 
	ArrowY->SetDirection(FVector(0.0f, 1.0f, 0.0f));//초
	ArrowZ->SetDirection(FVector(0.0f, 0.0f, 1.0f));//파

	ArrowX->SetColor(FVector(1.0f, 0.0f, 0.0f));
	ArrowY->SetColor(FVector(0.0f, 1.0f, 0.0f));
	ArrowZ->SetColor(FVector(0.0f, 0.0f, 1.0f));

	ArrowX->SetupAttachment(RootComponent);
	ArrowY->SetupAttachment(RootComponent);
	ArrowZ->SetupAttachment(RootComponent);

	ArrowX->SetRelativeScale({ 1, 3, 1 });
	ArrowY->SetRelativeScale({ 1, 3, 1 });
	ArrowZ->SetRelativeScale({ 1, 3, 1 });

	AddComponent(ArrowX);
	AddComponent(ArrowY);
	AddComponent(ArrowZ);
	GizmoArrowComponents.Add(ArrowX);
	GizmoArrowComponents.Add(ArrowY);
	GizmoArrowComponents.Add(ArrowZ);

	//======= Rotate Component 생성 =======
	RotateX = NewObject<UGizmoRotateComponent>();
	RotateY = NewObject<UGizmoRotateComponent>();
	RotateZ = NewObject<UGizmoRotateComponent>();

	RotateX->SetDirection(FVector(1.0f, 0.0f, 0.0f));
	RotateY->SetDirection(FVector(0.0f, 1.0f, 0.0f));
	RotateZ->SetDirection(FVector(0.0f, 0.0f, 1.0f));

	RotateX->SetColor(FVector(1.0f, 0.0f, 0.0f));
	RotateY->SetColor(FVector(0.0f, 1.0f, 0.0f));
	RotateZ->SetColor(FVector(0.0f, 0.0f, 1.0f));

	RotateX->SetupAttachment(RootComponent);
	RotateY->SetupAttachment(RootComponent);
	RotateZ->SetupAttachment(RootComponent);

	RotateX->SetRelativeScale({ 0.02f, 0.02f, 0.02f });
	RotateY->SetRelativeScale({ 0.02f, 0.02f, 0.02f });
	RotateZ->SetRelativeScale({ 0.02f, 0.02f, 0.02f });

	AddComponent(RotateX);
	AddComponent(RotateY);
	AddComponent(RotateZ);
	GizmoRotateComponents.Add(RotateX);
	GizmoRotateComponents.Add(RotateY);
	GizmoRotateComponents.Add(RotateZ);

	//======= Scale Component 생성 =======
	ScaleX = NewObject<UGizmoScaleComponent>();
	ScaleY = NewObject<UGizmoScaleComponent>();
	ScaleZ = NewObject<UGizmoScaleComponent>();

	ScaleX->SetDirection(FVector(1.0f, 0.0f, 0.0f));
	ScaleY->SetDirection(FVector(0.0f, 1.0f, 0.0f));
	ScaleZ->SetDirection(FVector(0.0f, 0.0f, 1.0f));

	ScaleX->SetColor(FVector(1.0f, 0.0f, 0.0f));
	ScaleY->SetColor(FVector(0.0f, 1.0f, 0.0f));
	ScaleZ->SetColor(FVector(0.0f, 0.0f, 1.0f));

	ScaleX->SetupAttachment(RootComponent);
	ScaleY->SetupAttachment(RootComponent);
	ScaleZ->SetupAttachment(RootComponent);

	ScaleX->SetRelativeScale({ 0.02f, 0.02f, 0.02f });
	ScaleY->SetRelativeScale({ 0.02f, 0.02f, 0.02f });
	ScaleZ->SetRelativeScale({ 0.02f, 0.02f, 0.02f });

	AddComponent(ScaleX);
	AddComponent(ScaleY);
	AddComponent(ScaleZ);
	GizmoScaleComponents.Add(ScaleX);
	GizmoScaleComponents.Add(ScaleY);
	GizmoScaleComponents.Add(ScaleZ);

	CurrentMode = EGizmoMode::Translate;

	// 매니저 참조 초기화
	SelectionManager = &USelectionManager::GetInstance();
	InputManager = &UInputManager::GetInstance();
	UIManager = &UUIManager::GetInstance();
}

void AGizmoActor::Tick(float DeltaSeconds)
{
	if (!SelectionManager) SelectionManager = &USelectionManager::GetInstance();
	if (!InputManager) InputManager = &UInputManager::GetInstance();
	if (!UIManager) UIManager = &UUIManager::GetInstance();

	// 컴포넌트 활성화 상태 업데이트    
	if (SelectionManager->HasSelection() && CameraActor)
	{
		AActor* SelectionActor = SelectionManager->GetSelectedActor();
		USceneComponent* SelectionComponent = SelectionManager->GetSelectedComponent();
		TargetComponent = SelectionComponent == nullptr ? SelectionActor->GetRootComponent() : SelectionComponent;

		// 기즈모 위치를 선택된 액터 위치로 업데이트
		if (TargetComponent)
		{
			SetSpaceWorldMatrix(CurrentSpace, TargetComponent);
			SetActorLocation(TargetComponent->GetWorldLocation());
		}
	}
	else
	{
		TargetComponent = nullptr;
	}
	UpdateComponentVisibility();
}
void AGizmoActor::Render(ACameraActor* Camera, FViewport* Viewport) {

	UpdateConstantScreenScale(Camera, Viewport);

	TArray<USceneComponent*>* Components = GetGizmoComponents();
	if (!Components) return;


	float ViewportAspectRatio = static_cast<float>(Viewport->GetSizeX()) / static_cast<float>(Viewport->GetSizeY());
	if (Viewport->GetSizeY() == 0) ViewportAspectRatio = 1.0f; // 0으로 나누기 방지

	EViewModeIndex ViewModeIndex = World->GetViewModeIndex();
	URenderer* Renderer = GetWorld()->GetRenderer();
	if (!USelectionManager::GetInstance().HasSelection()) return;
	FMatrix ViewMatrix = Camera->GetViewMatrix();
	FMatrix ProjectionMatrix = Camera->GetProjectionMatrix(ViewportAspectRatio, Viewport);
	FMatrix ModelMatrix;
	FVector rgb(1.0f, 1.0f, 1.0f);
	FVector2D ViewportSize(static_cast<float>(Viewport->GetSizeX()), static_cast<float>(Viewport->GetSizeY()));
	FVector2D ViewportOffset(static_cast<float>(Viewport->GetStartX()), static_cast<float>(Viewport->GetStartY()));
	FVector2D ViewportMousePos(static_cast<float>(UInputManager::GetInstance().GetMousePosition().X) + ViewportOffset.X,
		static_cast<float>(UInputManager::GetInstance().GetMousePosition().Y) + ViewportOffset.Y);



	for (int32 i = 0; i < Components->Num(); ++i)
	{
		USceneComponent* Component = (*Components)[i];
		if (!Component) continue;
		// 컴포넌트 활성 상태 확인
		if (UActorComponent* ActorComp = Cast<UActorComponent>(Component))
		{
			if (!ActorComp->IsActive()) continue;
		}

		ModelMatrix = Component->GetWorldMatrix();
		Renderer->UpdateConstantBuffer(ModelMatrix, ViewMatrix, ProjectionMatrix);


		if (GizmoAxis == i + 1)
		{
			Renderer->UpdateHighLightConstantBuffer(true, rgb, i + 1, 1, 0, 1);
		}
		else
		{
			Renderer->UpdateHighLightConstantBuffer(true, rgb, i + 1, 0, 0, 1);
		}


		if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
		{
			Renderer->SetViewModeType(EViewModeIndex::VMI_Unlit);
			Renderer->OMSetDepthStencilState(EComparisonFunc::Always);
			Renderer->OMSetBlendState(true); // 필요 시

			Primitive->Render(Renderer, ViewMatrix, ProjectionMatrix);
			// 상태 복구
			Renderer->OMSetBlendState(false);
			Renderer->OMSetDepthStencilState(EComparisonFunc::LessEqual);
			Renderer->SetViewModeType(ViewModeIndex);
		}
	}
	//Renderer->UpdateHighLightConstantBuffer(false, rgb, 0, 0, 0, 0);

	// 알파 블랜딩을 위한 blendstate
	Renderer->OMSetBlendState(true);
}

AGizmoActor::~AGizmoActor()
{
	// Components are centrally destroyed by AActor's destructor
	ArrowX = ArrowY = ArrowZ = nullptr;
	ScaleX = ScaleY = ScaleZ = nullptr;
	RotateX = RotateY = RotateZ = nullptr;
}

void AGizmoActor::SetMode(EGizmoMode NewMode)
{
	CurrentMode = NewMode;
}
EGizmoMode  AGizmoActor::GetMode()
{
	return CurrentMode;
}

void AGizmoActor::SetSpaceWorldMatrix(EGizmoSpace NewSpace, USceneComponent* PickedComponent)
{
	SetSpace(NewSpace);

	if (NewSpace == EGizmoSpace::World)
	{

		RootComponent->SetRelativeTransform(FTransform(PickedComponent->GetWorldLocation(), FQuat(), FVector(1, 1, 1)));
		// 월드 고정 → 기즈모 축은 항상 X/Y/Z
		   // 월드 고정 → 기즈모 축은 항상 X/Y/Z
		//Arrow = 모델이 -y를 바라보고 있음
		if (ArrowX) ArrowX->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0, 0, 90)));
		if (ArrowY) ArrowY->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0, 0, 180)));
		if (ArrowZ) ArrowZ->SetRelativeRotation(FQuat::MakeFromEuler(FVector(-90, 0, 0)));

		//Scale = 모델이 +z를 바라보고 있음
		if (ScaleX) ScaleX->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0, 90, 0)));
		if (ScaleY) ScaleY->SetRelativeRotation(FQuat::MakeFromEuler(FVector(-90, 0, 0)));
		if (ScaleZ) ScaleZ->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0, 0, 0)));

		if (RotateX) RotateX->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0, 90, 0)));
		if (RotateY) RotateY->SetRelativeRotation(FQuat::MakeFromEuler(FVector(90, 0, 0)));
		if (RotateZ) RotateZ->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0, 0, 0)));
	}
	else if (NewSpace == EGizmoSpace::Local)
	{
		if (!PickedComponent)
			return;

		// 타겟 액터 회전 가져오기
		RootComponent->SetRelativeLocation(PickedComponent->GetWorldLocation());

		FQuat TargetRot = PickedComponent->GetWorldRotation();

		//Arrow = 모델이 -y를 바라보고 있음
		if (ArrowX) ArrowX->SetRelativeRotation(TargetRot * FQuat::MakeFromEuler(FVector(0, 0, 90)));
		if (ArrowY) ArrowY->SetRelativeRotation(TargetRot * FQuat::MakeFromEuler(FVector(0, 0, 180)));
		if (ArrowZ) ArrowZ->SetRelativeRotation(TargetRot * FQuat::MakeFromEuler(FVector(-90, 0, 0)));

		//Scale = 모델이 +z를 바라보고 있음
		if (ScaleX) ScaleX->SetRelativeRotation(TargetRot * FQuat::MakeFromEuler(FVector(0, 90, 0)));
		if (ScaleY) ScaleY->SetRelativeRotation(TargetRot * FQuat::MakeFromEuler(FVector(-90, 0, 0)));
		if (ScaleZ) ScaleZ->SetRelativeRotation(TargetRot * FQuat::MakeFromEuler(FVector(0, 0, 0)));

		if (RotateX) RotateX->SetRelativeRotation(TargetRot * FQuat::MakeFromEuler(FVector(0, 90, 0)));
		if (RotateY) RotateY->SetRelativeRotation(TargetRot * FQuat::MakeFromEuler(FVector(90, 0, 0)));
		if (RotateZ) RotateZ->SetRelativeRotation(TargetRot * FQuat::MakeFromEuler(FVector(0, 0, 0)));
	}

}


void AGizmoActor::NextMode(EGizmoMode GizmoMode)
{
	CurrentMode = GizmoMode;
}

TArray<USceneComponent*>* AGizmoActor::GetGizmoComponents()
{
	switch (CurrentMode)
	{
	case EGizmoMode::Translate:
		return &GizmoArrowComponents;
	case EGizmoMode::Rotate:
		return &GizmoRotateComponents;
	case EGizmoMode::Scale:
		return &GizmoScaleComponents;
	}
	return nullptr;
}

EGizmoMode AGizmoActor::GetGizmoMode() const
{
	return CurrentMode;
}

// 개선된 축 투영 함수 - 수직 각도에서도 안정적
static FVector2D GetStableAxisDirection(const FVector& WorldAxis, const ACameraActor* Camera)
{
	const FVector CameraRight = Camera->GetRight();
	const FVector CameraUp = Camera->GetUp();
	const FVector CameraForward = Camera->GetForward();

	// 축과 카메라 forward의 각도 확인 (수직도 측정)
	float ForwardDot = FVector::Dot(WorldAxis.GetNormalized(), CameraForward.GetNormalized());
	float PerpendicularThreshold = 0.95f; // cos(약 18도)

	// 거의 수직인 경우 (축이 스크린과 거의 평행)
	if (std::fabs(ForwardDot) > PerpendicularThreshold)
	{
		// 가장 큰 성분을 가진 카메라 축 성분 사용
		float RightComponent = std::fabs(FVector::Dot(WorldAxis, CameraRight));
		float UpComponent = std::fabs(FVector::Dot(WorldAxis, CameraUp));

		if (RightComponent > UpComponent)
		{
			// Right 성분이 더 클 때: X축 우선
			float Sign = FVector::Dot(WorldAxis, CameraRight) > 0 ? 1.0f : -1.0f;
			return FVector2D(Sign, 0.0f);
		}
		else
		{
			// Up 성분이 더 클 때: Y축 우선 (DirectX는 Y가 아래쪽이므로 반전)
			float Sign = FVector::Dot(WorldAxis, CameraUp) > 0 ? -1.0f : 1.0f;
			return FVector2D(0.0f, Sign);
		}
	}

	// 일반적인 경우: 스크린 투영 사용
	float RightDot = FVector::Dot(WorldAxis, CameraRight);
	float UpDot = FVector::Dot(WorldAxis, CameraUp);

	// DirectX 스크린 좌표계 고려 (Y축 반전)
	FVector2D ScreenDirection = FVector2D(RightDot, -UpDot);

	// 안전한 정규화 (최소 길이 보장)
	float Length = ScreenDirection.Length();
	if (Length > 0.001f)
	{
		return ScreenDirection * (1.0f / Length);
	}

	// 예외 상황: 기본 X축 방향
	return FVector2D(1.0f, 0.0f);
}

void AGizmoActor::OnDrag(USceneComponent* TargetComponent, uint32 GizmoAxis, float MouseDeltaX, float MouseDeltaY, const ACameraActor* Camera, FViewport* Viewport)
{
	if (!TargetComponent || !Camera)
		return;

	if (GizmoAxis == 0)
	{
		return;
	}
	FVector2D MouseDelta = FVector2D(MouseDeltaX, MouseDeltaY);

	FVector Axis{};
	FVector GizmoPosition = GetActorLocation();
	
	switch (CurrentMode)
	{
		case EGizmoMode::Translate:
			if (CurrentSpace == EGizmoSpace::World)
			{
				switch (GizmoAxis)
				{
				case 1: Axis = FVector(1, 0, 0); break;
				case 2: Axis = FVector(0, 1, 0); break;
				case 3: Axis = FVector(0, 0, 1); break;
				}
			}
			else
			{
				switch (GizmoAxis)
				{
				case 1: Axis = TargetComponent->GetForward();   break; // Local X
				case 2: Axis = TargetComponent->GetRight(); break; // Local Y
				case 3: Axis = TargetComponent->GetUp();      break; // Local Z
				}
			}
			break;
	}
	
	

	// ────────────── 모드별 처리 ──────────────
	switch (CurrentMode)
	{
	case EGizmoMode::Translate:
	{
		if (CurrentSpace == EGizmoSpace::World)
		{
			switch (GizmoAxis)
			{
			case 1: Axis = FVector(1, 0, 0); break;
			case 2: Axis = FVector(0, 1, 0); break;
			case 3: Axis = FVector(0, 0, 1); break;
			}
		}
		else if (CurrentSpace == EGizmoSpace::Local)
		{
			switch (GizmoAxis)
			{
			case 1: Axis = TargetComponent->GetForward();   break; // Local X
			case 2: Axis = TargetComponent->GetRight(); break; // Local Y
			case 3: Axis = TargetComponent->GetUp();      break; // Local Z
			}
		}
		FVector2D ScreenAxis = GetStableAxisDirection(Axis, Camera);
		float px = (MouseDelta.X * ScreenAxis.X + MouseDelta.Y * ScreenAxis.Y);
		float h = Viewport ? static_cast<float>(Viewport->GetSizeY()) : UInputManager::GetInstance().GetScreenSize().Y;
		if (h <= 0.0f) h = 1.0f;
		float w = Viewport ? static_cast<float>(Viewport->GetSizeX()) : UInputManager::GetInstance().GetScreenSize().X;
		float aspect = w / h;
		FMatrix Proj = Camera->GetProjectionMatrix(aspect, Viewport);
		bool bOrtho = std::fabs(Proj.M[3][3] - 1.0f) < KINDA_SMALL_NUMBER;
		float worldPerPixel = 0.0f;
		if (bOrtho)
		{
			float halfH = 1.0f / Proj.M[1][1];
			worldPerPixel = (2.0f * halfH) / h;
		}
		else
		{
			float yScale = Proj.M[1][1];
			FVector camPos = Camera->GetActorLocation();
			FVector gizPos = GetActorLocation();
			FVector camF = Camera->GetForward();
			float z = FVector::Dot(gizPos - camPos, camF);
			if (z < 1.0f) z = 1.0f;
			worldPerPixel = (2.0f * z) / (h * yScale);
		}
		/*	float zoomFactor = Camera->GetCameraComponent()->GetZoomFactor();
	worldPerPixel *= zoomFactor;*/

		float Movement = px * worldPerPixel;
		FVector CurrentLocation = TargetComponent->GetWorldLocation();
		TargetComponent->SetWorldLocation(CurrentLocation + Axis * Movement);

		break;
	}
	case EGizmoMode::Scale:
	{
		// Determine axis for screen projection
		if (CurrentSpace == EGizmoSpace::World)
		{
			switch (GizmoAxis)
			{
			case 1: Axis = FVector(1, 0, 0); break;
			case 2: Axis = FVector(0, 1, 0); break;
			case 3: Axis = FVector(0, 0, 1); break;
			}
		}
		else if (CurrentSpace == EGizmoSpace::Local)
		{
			switch (GizmoAxis)
			{
			case 1: Axis = TargetComponent->GetForward();   break; // Local X
			case 2: Axis = TargetComponent->GetRight(); break; // Local Y
			case 3: Axis = TargetComponent->GetUp();      break; // Local Z
			}
		}

		FVector2D ScreenAxis = GetStableAxisDirection(Axis, Camera);
		float px = (MouseDelta.X * ScreenAxis.X + MouseDelta.Y * ScreenAxis.Y);
		float h = Viewport ? static_cast<float>(Viewport->GetSizeY()) : UInputManager::GetInstance().GetScreenSize().Y;
		if (h <= 0.0f) h = 1.0f;
		float w = Viewport ? static_cast<float>(Viewport->GetSizeX()) : UInputManager::GetInstance().GetScreenSize().X;
		float aspect = w / h;
		FMatrix Proj = Camera->GetProjectionMatrix(aspect, Viewport);
		bool bOrtho = std::fabs(Proj.M[3][3] - 1.0f) < KINDA_SMALL_NUMBER;
		float worldPerPixel = 0.0f;
		if (bOrtho)
		{
			float halfH = 1.0f / Proj.M[1][1];
			worldPerPixel = (2.0f * halfH) / h;
		}
		else
		{
			float yScale = Proj.M[1][1];
			FVector camPos = Camera->GetActorLocation();
			FVector gizPos = GetActorLocation();
			FVector camF = Camera->GetForward();
			float z = FVector::Dot(gizPos - camPos, camF);
			if (z < 1.0f) z = 1.0f;
			worldPerPixel = (2.0f * z) / (h * yScale);
		}
		float Movement = px * worldPerPixel;
		FVector NewScale = TargetComponent->GetRelativeScale();

		// Apply movement to the correct local axis based on which gizmo was dragged
		switch (GizmoAxis)
		{
		case 1: // X Axis
			NewScale.X += Movement;
			break;
		case 2: // Y Axis
			NewScale.Y += Movement;
			break;
		case 3: // Z Axis
			NewScale.Z += Movement;
			break;
		}

		TargetComponent->SetRelativeScale(NewScale);

		break;
	}
	case EGizmoMode::Rotate:
	{
		if (CurrentSpace == EGizmoSpace::World)
		{
			switch (GizmoAxis)
			{
			case 1: Axis = FVector(1, 0, 0); break;
			case 2: Axis = FVector(0, 1, 0); break;
			case 3: Axis = FVector(0, 0, 1); break;
			}
		}
		else if (CurrentSpace == EGizmoSpace::Local)
		{
			switch (GizmoAxis)
			{
			case 1: Axis = TargetComponent->GetParentForward();   break; // Local X
			case 2: Axis = TargetComponent->GetParentRight(); break; // Local Y
			case 3: Axis = TargetComponent->GetParentUp();      break; // Local Z
			}
		}
		float RotationSpeed = 0.5f;
		float DeltaAngleX = MouseDeltaX * RotationSpeed;
		float DeltaAngleY = MouseDeltaY * RotationSpeed;

		float Angle = DeltaAngleX + DeltaAngleY;

		// 로컬 모드일 경우 축을 Target 로컬 축으로
		FVector RotationAxis = Axis.GetSafeNormal();

		// = MakeQuatFromAxisAngle(RotationAxis.X, Angle);
		FQuat DeltaQuat{};
		FQuat RelativeRotation = TargetComponent->GetRelativeRotation();
		FVector TestEuler = RelativeRotation.ToEulerDegree();
		if (CurrentSpace == EGizmoSpace::World)
		{

			switch (GizmoAxis)
			{
			case 1: // X축 회전
			{
				// 마우스 X → 카메라 Up 축 기반
				FQuat RotByX = FQuat(FVector(-1, 0, 0), DeltaAngleX);
				// 마우스 Y → 카메라 Right 축 기반
				FQuat RotByY = FQuat(FVector(-1, 0, 0), DeltaAngleY);
				DeltaQuat = RotByX * RotByY;
				break;
			}
			case 2: // Y축 회전
			{
				FQuat RotByX = FQuat(FVector(0, -1, 0), DeltaAngleX);
				FQuat RotByY = FQuat(FVector(0, -1, 0), DeltaAngleY);
				DeltaQuat = RotByX * RotByY;
				break;
			}
			case 3: // Z축 회전
			{
				FQuat RotByX = FQuat(FVector(0, 0, -1), DeltaAngleX);
				FQuat RotByY = FQuat(FVector(0, 0, -1), DeltaAngleY);
				DeltaQuat = RotByX * RotByY;
				break;
			}
			}
			FQuat NewRot = DeltaQuat * RelativeRotation; // 월드 기준 회전
			TargetComponent->SetRelativeRotation(NewRot);
		}
		else
		{
			float RotationSpeed = 0.5f;
			float DeltaAngleX = MouseDeltaX * RotationSpeed;
			float DeltaAngleY = MouseDeltaY * RotationSpeed;

			float Angle = DeltaAngleX + DeltaAngleY;

			// 로컬 모드일 경우 축을 Target 로컬 축으로
			FVector RotationAxis = -Axis.GetSafeNormal();

			switch (GizmoAxis)
			{
			case 1: // X축 회전
			{
				// 마우스 X → 카메라 Up 축 기반
				FQuat RotByX = FQuat(RotationAxis, DeltaAngleX);
				// 마우스 Y → 카메라 Right 축 기반
				FQuat RotByY = FQuat(RotationAxis, DeltaAngleY);
				DeltaQuat = RotByX * RotByY;
				break;
			}
			case 2: // Y축 회전
			{
				FQuat RotByX = FQuat(RotationAxis, DeltaAngleX);
				FQuat RotByY = FQuat(RotationAxis, DeltaAngleY);
				DeltaQuat = RotByX * RotByY;
				break;
			}
			case 3: // Z축 회전
			{
				FQuat RotByX = FQuat(RotationAxis, DeltaAngleX);
				FQuat RotByY = FQuat(RotationAxis, DeltaAngleY);
				DeltaQuat = RotByX * RotByY;
				break;
			}
			}

			FQuat NewRot = DeltaQuat * RelativeRotation; // 월드 기준 회전
			TargetComponent->SetRelativeRotation(NewRot);
			break;
		}
	}
	}
}



void AGizmoActor::UpdateGizmoPosition()
{
	if (!TargetComponent) return;
	SetActorLocation(TargetComponent->GetWorldLocation());
}

void AGizmoActor::ProcessGizmoInteraction(ACameraActor* Camera, FViewport* Viewport, float MousePositionX, float MousePositionY)
{
	if (!TargetComponent || !Camera) return;


	UpdateConstantScreenScale(Camera, Viewport);

	ProcessGizmoModeSwitch();

	// 기즈모 드래그
	ProcessGizmoDragging(Camera, Viewport, MousePositionX, MousePositionY);

	ProcessGizmoHovering(Camera, Viewport, MousePositionX, MousePositionY);

}

void AGizmoActor::ProcessGizmoHovering(ACameraActor* Camera, FViewport* Viewport, float MousePositionX, float MousePositionY)
{
	if (!Camera) return;

	FVector2D ViewportSize(static_cast<float>(Viewport->GetSizeX()), static_cast<float>(Viewport->GetSizeY()));
	FVector2D ViewportOffset(static_cast<float>(Viewport->GetStartX()), static_cast<float>(Viewport->GetStartY()));
	FVector2D ViewportMousePos(static_cast<float>(MousePositionX) + ViewportOffset.X,
		static_cast<float>(MousePositionY) + ViewportOffset.Y);
	if (!bIsDragging)
		GizmoAxis = CPickingSystem::IsHoveringGizmoForViewport(this, Camera, ViewportMousePos, ViewportSize, ViewportOffset, Viewport);

	if (GizmoAxis > 0)//기즈모 축이 0이상이라면 선택 된것 
	{
		bIsHovering = true;
	}
	else
	{
		bIsHovering = false;
	}

}

void AGizmoActor::ProcessGizmoDragging(ACameraActor* Camera, FViewport* Viewport, float MousePositionX, float MousePositionY)
{
	if (!TargetComponent || !Camera) return;

	if (InputManager->IsMouseButtonDown(LeftButton))
	{
		FVector2D MouseDelta = InputManager->GetMouseDelta();
		if ((MouseDelta.X * MouseDelta.X + MouseDelta.Y * MouseDelta.Y) > 0.0f)
		{
			OnDrag(TargetComponent, GizmoAxis, MouseDelta.X, MouseDelta.Y, CameraActor, Viewport);
			bIsDragging = true;
			SetActorLocation(TargetComponent->GetWorldLocation());
		}
	}
	if (InputManager->IsMouseButtonReleased(LeftButton))
	{
		bIsDragging = false;
		GizmoAxis = 0;
		SetSpaceWorldMatrix(CurrentSpace, TargetComponent);
	}
}

void AGizmoActor::ProcessGizmoModeSwitch()
{
	// 스페이스 키로 기즈모 모드 전환
	if (InputManager->IsKeyPressed(VK_SPACE))
	{
		int GizmoModeIndex = static_cast<int>(GetMode());
		GizmoModeIndex = (GizmoModeIndex + 1) % 3;  // 3 = enum 개수
		EGizmoMode NewGizmoMode = static_cast<EGizmoMode>(GizmoModeIndex);
		NextMode(NewGizmoMode);
	}
}

void AGizmoActor::UpdateComponentVisibility()
{
	// 선택된 액터가 없으면 모든 기즈모 컴포넌트를 비활성화
	bool bHasSelection = SelectionManager && SelectionManager->HasSelection();

	// Arrow Components (Translate 모드)
	bool bShowArrows = bHasSelection && (CurrentMode == EGizmoMode::Translate);
	if (ArrowX) ArrowX->SetActive(bShowArrows);
	if (ArrowY) ArrowY->SetActive(bShowArrows);
	if (ArrowZ) ArrowZ->SetActive(bShowArrows);

	// Rotate Components (Rotate 모드)
	bool bShowRotates = bHasSelection && (CurrentMode == EGizmoMode::Rotate);
	if (RotateX) RotateX->SetActive(bShowRotates);
	if (RotateY) RotateY->SetActive(bShowRotates);
	if (RotateZ) RotateZ->SetActive(bShowRotates);

	// Scale Components (Scale 모드)
	bool bShowScales = bHasSelection && (CurrentMode == EGizmoMode::Scale);
	if (ScaleX) ScaleX->SetActive(bShowScales);
	if (ScaleY) ScaleY->SetActive(bShowScales);
	if (ScaleZ) ScaleZ->SetActive(bShowScales);
}

void AGizmoActor::UpdateConstantScreenScale(ACameraActor* Camera, FViewport* Viewport)
{
	if (!Camera || !Viewport) return;
	float h = static_cast<float>(Viewport->GetSizeY());
	if (h <= 0.0f) h = 1.0f;
	float w = static_cast<float>(Viewport->GetSizeX());
	float aspect = w / h;
	FMatrix Proj = Camera->GetProjectionMatrix(aspect, Viewport);
	bool bOrtho = std::fabs(Proj.M[3][3] - 1.0f) < KINDA_SMALL_NUMBER;
	float targetPx = 30.0f;
	float scale = 1.0f;
	if (bOrtho)
	{
		float halfH = 1.0f / Proj.M[1][1];
		scale = (targetPx * 2.0f * halfH) / h;
	}
	else
	{
		float yScale = Proj.M[1][1];
		FVector camPos = Camera->GetActorLocation();
		FVector gizPos = GetActorLocation();
		FVector camF = Camera->GetForward();
		float z = FVector::Dot(gizPos - camPos, camF);
		if (z < 1.0f) z = 1.0f;
		scale = (targetPx * 2.0f * z) / (h * yScale);
	}
	if (scale < 0.001f) scale = 0.001f;
	if (scale > 10000.0f) scale = 10000.0f;
	SetActorScale(FVector(scale, scale, scale));
}

void AGizmoActor::OnDrag(USceneComponent* TargetComponent, uint32 GizmoAxis, float MouseDeltaX, float MouseDeltaY, const ACameraActor* Camera)
{
	OnDrag(TargetComponent, GizmoAxis, MouseDeltaX, MouseDeltaY, Camera, nullptr);
}
