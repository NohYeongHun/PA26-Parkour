#pragma once

#include "Base.h"

NS_BEGIN(Engine)

namespace BoneReadbackProfiler
{
	// 프레임 단위 측정 API
	ENGINE_DLL void BeginFrame();
	ENGINE_DLL void EndFrame();

	// Readback_BoneMatrices 한 번 호출에 대한 시간(ms) 누적
	ENGINE_DLL void AddReadbackMs(double ms);

	// 녹화 세션 제어 (핫키에서 사용)
	ENGINE_DLL void BeginRecordingSession();
	ENGINE_DLL void EndRecordingSessionAndExportCsv(const char* pPathOrNull);
	ENGINE_DLL _bool IsRecording();

	// 더블 버퍼링 모드 제어 (플레이 중 토글)
	ENGINE_DLL void SetUseDoubleBuffering(_bool bEnable);
	ENGINE_DLL _bool GetUseDoubleBuffering();

	// 선택: ImGui 디버그 창 (상태/최근 프레임 확인용)
	ENGINE_DLL void DrawImGui();
}

NS_END

