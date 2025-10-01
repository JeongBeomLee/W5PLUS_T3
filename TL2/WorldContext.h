#pragma once
#include "Enums.h"

class UWorld;

/**
 * FWorldContext
 * - 여러 월드(Editor, PIE, Game 등)를 관리하기 위한 컨텍스트
 * - 각 월드의 타입과 상태를 추적
 */
struct FWorldContext
{
    UWorld* World = nullptr;
    EWorldType WorldType = EWorldType::None;

    // PIE 월드의 경우 원본 Editor 월드 참조
    UWorld* OwningEditorWorld = nullptr;

    FWorldContext() = default;
    FWorldContext(UWorld* InWorld, EWorldType InType)
        : World(InWorld), WorldType(InType)
    {
    }

    UWorld* GetWorld() const { return World; }
    EWorldType GetWorldType() const { return WorldType; }

    bool IsEditorWorld() const { return WorldType == EWorldType::Editor; }
    bool IsPIEWorld() const { return WorldType == EWorldType::PIE; }
    bool IsGameWorld() const { return WorldType == EWorldType::Game; }
};