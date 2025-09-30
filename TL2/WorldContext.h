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

    FWorldContext() = default;
    FWorldContext(UWorld* InWorld, EWorldType InType)
        : World(InWorld), WorldType(InType)
    {
    }

    UWorld* GetWorld() const { return World; }
    EWorldType GetWorldType() const { return WorldType; }
};