#pragma once
#include "Widget.h"
#include "../../ImGui/imgui.h"

/**
 * @brief Frame과 관련된 내용을 제공하는 UI Widget
 */
class UFPSWidget :
	public UWidget
{
public:
	DECLARE_CLASS(UFPSWidget, UWidget)

	void Initialize() override;
	void Update() override;
	void RenderWidget() override;

	static ImVec4 GetFPSColor(float InFPS);

	// GameTime 초기화 (PIE 종료 시 호출)
	void ResetGameTime();

	// Special Member Function
	UFPSWidget();
	~UFPSWidget() override;

private:
	float FrameTimeHistory[60] = {};
	int32 FrameTimeIndex = 0;
	float AverageFrameTime = 0.0f;

	float CurrentFPS = 0.0f;
	float MinFPS = 999.0f;
	float MaxFPS = 0.0f;

	float TotalGameTime = 0.0f;
	float CurrentDeltaTime = 0.0f;

	// 출력을 위한 변수
	float PreviousTime = 0.0f;
	float CurrentTime = 0.0f;
	float PrintFPS;
	float PrintDeltaTime;
	bool bShowGraph = false;
};
