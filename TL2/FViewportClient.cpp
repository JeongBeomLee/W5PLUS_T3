#include "pch.h"
#include "FViewportClient.h"
#include "FViewport.h"
#include "CameraComponent.h"
#include "CameraActor.h"
#include "World.h"
#include "Picking.h"
#include "SelectionManager.h"
#include "GizmoActor.h"

FVector FViewportClient::CameraAddPosition{};

FViewportClient::FViewportClient()
{
	ViewportType = EViewportType::Perspective;
	// 직교 뷰별 기본 카메라 설정
	Camera = NewObject<ACameraActor>();
	ViewPortCamera = Camera;
	SetupCameraMode();
}

FViewportClient::~FViewportClient()
{
}

void FViewportClient::Tick(float DeltaTime) 
{
	if (PerspectiveCameraInput)
	{
		Camera->ProcessEditorCameraInput(DeltaTime);
	}

	MouseWheel(DeltaTime);
}

void FViewportClient::Draw(FViewport* Viewport)
{
	if (!Viewport || !World) return;

	// 뷰포트의 실제 크기로 aspect ratio 계산
	float ViewportAspectRatio = static_cast<float>(Viewport->GetSizeX()) / static_cast<float>(Viewport->GetSizeY());
	if (Viewport->GetSizeY() == 0) ViewportAspectRatio = 1.0f; // 0으로 나누기 방지

	AGizmoActor* GizmoActor = World->GetGizmoActor();

	switch (ViewportType)
	{
	case EViewportType::Perspective:
	{
		ACameraActor* MainCamera = World->GetCameraActor();
		MainCamera->GetCameraComponent()->SetProjectionMode(ECameraProjectionMode::Perspective);
		if (Viewport->GetMainViewport())
		{
			Camera = MainCamera;
		}
		Camera->GetCameraComponent()->SetProjectionMode(ECameraProjectionMode::Perspective);
		PerspectiveCameraPosition = Camera->GetActorLocation();
		PerspectiveCameraRotation = Camera->GetActorRotation();
		PerspectiveCameraFov = Camera->GetCameraComponent()->GetFOV();
		if (World)
		{
			World->SetViewModeIndex(ViewModeIndex);
			World->RenderViewports(Camera, Viewport);

			if (GizmoActor)
			{
				GizmoActor->Render(Camera, Viewport);
			}
		}
		break;
	}
	case EViewportType::Orthographic_Top:
	case EViewportType::Orthographic_Front:
	case EViewportType::Orthographic_Left:
	case EViewportType::Orthographic_Back:
	case EViewportType::Orthographic_Bottom:
	case EViewportType::Orthographic_Right:
	{
		Camera = ViewPortCamera;
		Camera->GetCameraComponent()->SetProjectionMode(ECameraProjectionMode::Orthographic);
		SetupCameraMode();
		// 월드의 모든 액터들을 렌더링
		if (World)
		{
			World->SetViewModeIndex(ViewModeIndex);
			World->RenderViewports(Camera, Viewport);
			
			if (GizmoActor)
			{
				GizmoActor->Render(Camera, Viewport);
			}
		}
		break;
	}
	}
}

void FViewportClient::SetupCameraMode()
{
	Camera = ViewPortCamera;
	switch (ViewportType)
	{
	case EViewportType::Perspective:
		Camera->SetActorLocation(PerspectiveCameraPosition);
		Camera->SetActorRotation(PerspectiveCameraRotation);
		Camera->GetCameraComponent()->SetFOV(PerspectiveCameraFov);
		break;

	case EViewportType::Orthographic_Top:
		Camera->SetActorLocation({ CameraAddPosition.X, CameraAddPosition.Y, 1000 });
		Camera->SetActorRotation(FQuat::MakeFromEuler({ 0, 90, 0 }));
		Camera->GetCameraComponent()->SetFOV(100);
		break;

	case EViewportType::Orthographic_Bottom:
		Camera->SetActorLocation({ CameraAddPosition.X, CameraAddPosition.Y, -1000 });
		Camera->SetActorRotation(FQuat::MakeFromEuler({ 0, -90, 0 }));
		Camera->GetCameraComponent()->SetFOV(100);
		break;

	case EViewportType::Orthographic_Left:
		Camera->SetActorLocation({ CameraAddPosition.X, 1000 , CameraAddPosition.Z });
		Camera->SetActorRotation(FQuat::MakeFromEuler({ 0, 0, -90 }));
		Camera->GetCameraComponent()->SetFOV(100);
		break;

	case EViewportType::Orthographic_Right:
		Camera->SetActorLocation({ CameraAddPosition.X, -1000, CameraAddPosition.Z });
		Camera->SetActorRotation(FQuat::MakeFromEuler({ 0, 0, 90 }));
		Camera->GetCameraComponent()->SetFOV(100);
		break;

	case EViewportType::Orthographic_Front:
		Camera->SetActorLocation({ -1000 , CameraAddPosition.Y, CameraAddPosition.Z });
		Camera->SetActorRotation(FQuat::MakeFromEuler({ 0, 0, 0 }));
		Camera->GetCameraComponent()->SetFOV(100);
		break;

	case EViewportType::Orthographic_Back:
		Camera->SetActorLocation({ 1000 , CameraAddPosition.Y, CameraAddPosition.Z });
		Camera->SetActorRotation(FQuat::MakeFromEuler({ 0, 0, 180 }));
		Camera->GetCameraComponent()->SetFOV(100);
		break;
	}
}

void FViewportClient::MouseMove(FViewport* Viewport, int32 X, int32 Y)
{
	AGizmoActor* GizmoActor = World->GetGizmoActor();

	if (GizmoActor)
	{
		GizmoActor->ProcessGizmoInteraction(Camera, Viewport, static_cast<float>(X), static_cast<float>(Y));
	}

	bool bCanOrthographicCameraMove = !bIsMouseButtonDown && bIsMouseRightButtonDown && (World->IsPIEWorld() || !GizmoActor->GetbIsHovering());
	if (bCanOrthographicCameraMove) // 직교투영이고 마우스 버튼이 눌려있을 때
	{
		if (ViewportType != EViewportType::Perspective) 
		{
			int32 DeltaX = X - MouseLastX;
			int32 DeltaY = Y - MouseLastY;

			if (Camera && (DeltaX != 0 || DeltaY != 0))
			{
				// 기준 픽셀→월드 스케일
				const float BasePixelToWorld = 0.05f;

				// 줌인(값↑)일수록 더 천천히 움직이도록 역수 적용
				float Zoom = Camera->GetCameraComponent()->GetZoomFactor();
				Zoom = (Zoom <= 0.f) ? 1.f : Zoom; // 안전장치
				const float pixelToWorld = BasePixelToWorld * Zoom;

				const FVector Right = Camera->GetRight();
				const FVector Up = Camera->GetUp();

				CameraAddPosition = 
					CameraAddPosition
					- Right * (DeltaX * pixelToWorld)
					+ Up * (DeltaY * pixelToWorld);

				SetupCameraMode();
			}

			MouseLastX = X;
			MouseLastY = Y;
		}
		else if (ViewportType == EViewportType::Perspective) 
		{
			PerspectiveCameraInput = true;
		}
	}

}
void FViewportClient::MouseButtonDown(FViewport* Viewport, int32 X, int32 Y, int32 Button)
{
	if (!Viewport || !World) // Only handle left mouse button
		return;

	// PIE 모드에서는 피킹을 비활성화 (Editor 전용 기능)
	if (World->IsPIEWorld())
	{
		if (Button == 1)
		{
			bIsMouseRightButtonDown = true;
			MouseLastX = X;
			MouseLastY = Y;
		}
		return;
	}

	// Get viewport size
	FVector2D ViewportSize(static_cast<float>(Viewport->GetSizeX()), static_cast<float>(Viewport->GetSizeY()));
	FVector2D ViewportOffset(static_cast<float>(Viewport->GetStartX()), static_cast<float>(Viewport->GetStartY()));

	// X, Y are already local coordinates within the viewport, convert to global coordinates for picking
	FVector2D ViewportMousePos(static_cast<float>(X) + ViewportOffset.X, static_cast<float>(Y) + ViewportOffset.Y);

	AActor* PickedActor = nullptr;
	TArray<AActor*> AllActors = World->GetActors();
	AGizmoActor* GizmoActor = World->GetGizmoActor();

	if (Button == 0) {
		bIsMouseButtonDown = true;
		// 뷰포트의 실제 aspect ratio 계산
		float PickingAspectRatio = ViewportSize.X / ViewportSize.Y;
		if (ViewportSize.Y == 0) PickingAspectRatio = 1.0f; // 0으로 나누기 방지
		if (GizmoActor && GizmoActor->GetbIsHovering())
		{
			return;
		}
		PickedActor = CPickingSystem::PerformGlobalBVHPicking(AllActors, Camera, ViewportMousePos, ViewportSize, ViewportOffset, PickingAspectRatio, Viewport);


		if (PickedActor)
		{
			USelectionManager::GetInstance().SelectActor(PickedActor);
			UUIManager::GetInstance().SetPickedActor(PickedActor);
			if (GizmoActor)
			{
				World->GetGizmoActor()->SetTargetActor(PickedActor);
				World->GetGizmoActor()->SetActorLocation(PickedActor->GetActorLocation());
			}
		}
		else
		{
			UUIManager::GetInstance().ResetPickedActor();
			// Clear selection if nothing was picked
			USelectionManager::GetInstance().ClearSelection();
		}
	}
	else if (Button == 1) {//우클릭시
		bIsMouseRightButtonDown = true;
		MouseLastX = X;
		MouseLastY = Y;

	}

}

void FViewportClient::MouseButtonUp(FViewport* Viewport, int32 X, int32 Y, int32 Button)
{
	if (Button == 0) // Left mouse button
	{
		bIsMouseButtonDown = false;
	}
	else {
		bIsMouseRightButtonDown = false;
		PerspectiveCameraInput = false;
	}
}

void FViewportClient::MouseWheel(float DeltaSeconds)
{
	if (!Camera) return;

	UCameraComponent* CameraComponent = Camera->GetCameraComponent();
	if (!CameraComponent) return;
	float WheelDelta = UInputManager::GetInstance().GetMouseWheelDelta();

	float zoomFactor = CameraComponent->GetZoomFactor();
	zoomFactor *= (1.0f - WheelDelta * DeltaSeconds * 3.0f);

	CameraComponent->SetZoomFactor(zoomFactor);

}

