#pragma once
#include "Object.h"
#include "WorldContext.h"

class UWorld;

/**
 * UEditorEngine
 * - 여러 WorldContext를 관리하는 엔진 클래스
 * - Editor 월드와 PIE 월드를 동시에 관리
 * - WorldType에 따라 다른 Tick 로직 적용
 */
class UEditorEngine : public UObject
{
public:
    DECLARE_CLASS(UEditorEngine, UObject)

    UEditorEngine();
    ~UEditorEngine() override;

    // WorldContext 관리
    FWorldContext* CreateWorldContext(UWorld* World, EWorldType WorldType);
    FWorldContext* GetWorldContext(UWorld* World);
    FWorldContext* GetWorldContextByType(EWorldType WorldType);
    void RemoveWorldContext(UWorld* World);

    // Editor 월드 접근
    FWorldContext& GetEditorWorldContext();
    UWorld* GetEditorWorld() const;

    // PIE 월드 접근
    UWorld* GetPIEWorld() const;

    // 활성화된 월드 (PIE 실행 중이면 PIE, 아니면 Editor)
    UWorld* GetActiveWorld() const;

    // 메인 Tick
    virtual void Tick(float DeltaSeconds);

    // PIE 종료 요청
    void RequestEndPIE();
    bool IsPIEEnding() const { return bPendingEndPIE; }

private:
    // WorldType별 Tick 처리
    void TickEditorWorld(UWorld* World, float DeltaSeconds);
    void TickPIEWorld(UWorld* World, float DeltaSeconds);

    // PIE 정리 내부 함수
    void CleanupPIE();

private:
    TArray<FWorldContext> WorldContexts;
    bool bPendingEndPIE = false;
};
