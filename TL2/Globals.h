#pragma once

// Forward Declarations
class UWorld;
class UEditorEngine;

/**
 * 전역 포인터들
 * - GWorld: 현재 활성화된 월드 (Editor 또는 PIE)
 * - GEditor: EditorEngine 싱글톤 포인터
 */

// 현재 활성화된 월드 (PIE 실행 중이면 PIE 월드, 아니면 Editor 월드)
extern UWorld* GWorld;

// EditorEngine
extern UEditorEngine* GEditor;
