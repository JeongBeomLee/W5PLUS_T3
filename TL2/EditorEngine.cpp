#include "pch.h"
#include "EditorEngine.h"
#include "World.h"
#include "Actor.h"
#include "Level.h"
#include "GizmoActor.h"
#include "ObjectFactory.h"
#include "SMultiViewportWindow.h"
#include "InputManager.h"
#include "UI/UIManager.h"

UEditorEngine::UEditorEngine()
{
}

UEditorEngine::~UEditorEngine()
{
}

FWorldContext* UEditorEngine::CreateWorldContext(UWorld* World, EWorldType WorldType)
{
    if (!World)
    {
        return nullptr;
    }

    // 이미 존재하는지 확인
    for (FWorldContext& Context : WorldContexts)
    {
        if (Context.World == World)
        {
            return &Context;
        }
    }

    // 새로운 WorldContext 생성
    FWorldContext NewContext(World, WorldType);
    WorldContexts.Add(NewContext);

    return &WorldContexts[WorldContexts.Num() - 1];
}

FWorldContext* UEditorEngine::GetWorldContext(UWorld* World)
{
    if (!World)
    {
        return nullptr;
    }

    for (FWorldContext& Context : WorldContexts)
    {
        if (Context.World == World)
        {
            return &Context;
        }
    }

    return nullptr;
}

FWorldContext* UEditorEngine::GetWorldContextByType(EWorldType WorldType)
{
    for (FWorldContext& Context : WorldContexts)
    {
        if (Context.WorldType == WorldType)
        {
            return &Context;
        }
    }

    return nullptr;
}

void UEditorEngine::RemoveWorldContext(UWorld* World)
{
    if (!World)
    {
        return;
    }

    for (int32 i = 0; i < WorldContexts.Num(); ++i)
    {
        if (WorldContexts[i].World == World)
        {
            WorldContexts.RemoveAt(i);
            return;
        }
    }
}

FWorldContext& UEditorEngine::GetEditorWorldContext()
{
    FWorldContext* Context = GetWorldContextByType(EWorldType::Editor);
    if (Context)
    {
        return *Context;
    }

    // Editor 월드가 없으면 첫 번째 WorldContext 반환 (안전장치)
    static FWorldContext EmptyContext;
    return EmptyContext;
}

UWorld* UEditorEngine::GetEditorWorld() const
{
    for (const FWorldContext& Context : WorldContexts)
    {
        if (Context.IsEditorWorld())
        {
            return Context.World;
        }
    }
    return nullptr;
}

UWorld* UEditorEngine::GetPIEWorld() const
{
    for (const FWorldContext& Context : WorldContexts)
    {
        if (Context.IsPIEWorld())
        {
            return Context.World;
        }
    }
    return nullptr;
}

UWorld* UEditorEngine::GetActiveWorld() const
{
    // PIE 월드가 있으면 PIE 월드 반환
    UWorld* PIEWorld = GetPIEWorld();
    if (PIEWorld)
    {
        return PIEWorld;
    }

    // 없으면 Editor 월드 반환
    return GetEditorWorld();
}

void UEditorEngine::Tick(float DeltaSeconds)
{
    // PIE 종료 요청이 있으면 현재 프레임 완료 후 정리
    if (bPendingEndPIE)
    {
        CleanupPIE();
        bPendingEndPIE = false;
        return; // 이번 틱은 스킵
    }

    for (FWorldContext& WorldContext : WorldContexts)
    {
        UWorld* World = WorldContext.GetWorld();
        if (!World) continue;

        switch (WorldContext.GetWorldType())
        {
        case EWorldType::Editor:
            TickEditorWorld(World, DeltaSeconds);
            break;

        case EWorldType::PIE:
            TickPIEWorld(World, DeltaSeconds);
            break;
        }
    }
}

void UEditorEngine::TickEditorWorld(UWorld* World, float DeltaSeconds)
{
    if (!World) return;

    // Level의 액터들 Tick (Editor 전용)
    if (ULevel* Level = World->GetLevel())
    {
        for (AActor* Actor : Level->GetActors())
        {
            if (Actor && Actor->ShouldTickInEditor())
            {
                Actor->Tick(DeltaSeconds);
            }
        }
    }

    // Engine Actors (Camera, Grid 등)
    const TArray<AActor*>& EngineActors = World->GetEngineActors();
    for (AActor* EngineActor : EngineActors)
    {
        if (EngineActor && EngineActor->ShouldTickInEditor())
        {
            EngineActor->Tick(DeltaSeconds);
        }
    }

    // GizmoActor는 항상 Tick (Editor 전용)
    if (AGizmoActor* Gizmo = World->GetGizmoActor())
    {
        Gizmo->Tick(DeltaSeconds);
    }

    // Viewport 입력 처리 (Editor에서만)
    World->ProcessViewportInput();

    // MultiViewport 업데이트
    if (SMultiViewportWindow* MultiViewport = World->GetMultiViewportWindow())
    {
        MultiViewport->OnUpdate(DeltaSeconds);
    }

    // InputManager와 UIManager 업데이트
    UInputManager::GetInstance().Update();
    UUIManager::GetInstance().Update(DeltaSeconds);
}

void UEditorEngine::TickPIEWorld(UWorld* World, float DeltaSeconds)
{
    if (!World) return;

    // Level의 액터들 Tick (PIE/Game 모드 동일하게 처리하면 될듯)
    if (ULevel* Level = World->GetLevel())
    {
        for (AActor* Actor : Level->GetActors())
        {
            if (Actor && Actor->CanTickInPlayMode())
            {
                Actor->Tick(DeltaSeconds);
            }
        }
    }

    // PIE에서는 GizmoActor Tick 안 함 (nullptr이므로)
    // Viewport 입력 처리 (PIE에서도 카메라 조작 가능하도록 설정함)
    World->ProcessViewportInput();

    // MultiViewport 업데이트
    if (SMultiViewportWindow* MultiViewport = World->GetMultiViewportWindow())
    {
        MultiViewport->OnUpdate(DeltaSeconds);
    }

    // InputManager와 UIManager 업데이트 (PIE 중에도 UI 표시)
    UInputManager::GetInstance().Update();
    UUIManager::GetInstance().Update(DeltaSeconds);
}

void UEditorEngine::RequestEndPIE()
{
    bPendingEndPIE = true;
}

void UEditorEngine::CleanupPIE()
{
    UWorld* PIEWorld = GetPIEWorld();
    if (!PIEWorld)
    {
        return;
    }

    UWorld* EditorWorld = GetEditorWorld();

    // ViewportClient를 Editor 월드로 먼저 복원 (SViewportWindow에서 처리하도록 남겨둠)
    // 하지만 여기서는 GWorld만 복원
    extern UWorld* GWorld;
    GWorld = EditorWorld;

    // PIE 월드 정리
    PIEWorld->CleanupWorld();

    // PIE 월드를 WorldContext에서 제거
    RemoveWorldContext(PIEWorld);

    // PIE 월드 삭제
    ObjectFactory::DeleteObject(PIEWorld);

    UE_LOG("PIE Stopped (Deferred)\n");
}

