#include "EnginePch.h"
#include "BoneReadbackProfiler.h"

#include <sstream>
#include <iomanip>

NS_BEGIN(Engine)

namespace
{
	LARGE_INTEGER g_FrameStartQpc = {};
	_bool         g_HasFrameStart = false;

	double        g_ReadbackMsThisFrame = 0.0;

	double        g_LastFrameReadbackMs = 0.0;
	double        g_LastFrameWallMs     = 0.0;

	_bool         g_IsRecording         = false;
	_uint         g_FrameCounter        = 0;

	// 더블 버퍼링 모드 (기본 ON)
	_bool         g_UseDoubleBuffering  = true;

	vector<string> g_CsvRows;

	double QpcToMs(const LARGE_INTEGER& begin, const LARGE_INTEGER& end)
	{
		LARGE_INTEGER freq = {};
		QueryPerformanceFrequency(&freq);
		if (0 == freq.QuadPart)
			return 0.0;

		return static_cast<double>(end.QuadPart - begin.QuadPart) * 1000.0
			/ static_cast<double>(freq.QuadPart);
	}

	string MakeDefaultCsvPath()
	{
		// 예: BoneReadback_20260403_153045.csv
		std::time_t t = std::time(nullptr);
		std::tm     tmLocal{};

		// Windows용 로컬타임 변환
		localtime_s(&tmLocal, &t);

		std::ostringstream oss;
		oss << "BoneReadback_"
		    << std::put_time(&tmLocal, "%Y%m%d_%H%M%S")
		    << ".csv";

		return oss.str();
	}
}

namespace BoneReadbackProfiler
{
	void BeginFrame()
	{
		QueryPerformanceCounter(&g_FrameStartQpc);
		g_HasFrameStart        = true;
		g_ReadbackMsThisFrame  = 0.0;
	}

	void EndFrame()
	{
		if (false == g_HasFrameStart)
			return;

		LARGE_INTEGER end{};
		QueryPerformanceCounter(&end);

		const double frameMs = QpcToMs(g_FrameStartQpc, end);
		g_HasFrameStart      = false;

		++g_FrameCounter;

		g_LastFrameReadbackMs = g_ReadbackMsThisFrame;
		g_LastFrameWallMs     = frameMs;

		if (g_IsRecording)
		{
			std::ostringstream oss;
			oss << g_FrameCounter << ','
			    << (g_UseDoubleBuffering ? "DoubleBuffer ON" : "DubleBuffer_OFF") << ','
			    << g_LastFrameReadbackMs << ','
			    << g_LastFrameWallMs;

			g_CsvRows.push_back(oss.str());
		}
	}

	void AddReadbackMs(double ms)
	{
		g_ReadbackMsThisFrame += ms;
	}

	void BeginRecordingSession()
	{
		g_CsvRows.clear();
		g_IsRecording = true;
	}

	void EndRecordingSessionAndExportCsv(const char* pPathOrNull)
	{
		if (false == g_IsRecording)
			return;

		g_IsRecording = false;

		if (g_CsvRows.empty())
			return;

		string path;
		if (nullptr != pPathOrNull && '\0' != pPathOrNull[0])
			path = pPathOrNull;
		else
			path = MakeDefaultCsvPath();

		std::ofstream ofs(path);
		if (!ofs.is_open())
			return;

		ofs << "frame,mode,readback_ms,frame_ms\n";
		for (const string& row : g_CsvRows)
			ofs << row << '\n';

		ofs.close();
	}

	_bool IsRecording()
	{
		return g_IsRecording;
	}

	void SetUseDoubleBuffering(_bool bEnable)
	{
		g_UseDoubleBuffering = bEnable;
	}

	_bool GetUseDoubleBuffering()
	{
		return g_UseDoubleBuffering;
	}

	void DrawImGui()
	{
		if (nullptr == ImGui::GetCurrentContext())
			return;

		if (ImGui::Begin("Bone Readback (Model Double Buffering)"))
		{
			ImGui::Text("Mode: %s", g_UseDoubleBuffering ? "DB_ON" : "DB_OFF");
			ImGui::Text("Recording: %s", g_IsRecording ? "YES" : "NO");
			ImGui::Separator();
			ImGui::Text("Last frame readback: %.4f ms", g_LastFrameReadbackMs);
			ImGui::Text("Last frame time:     %.4f ms", g_LastFrameWallMs);
			ImGui::Text("Recorded rows:       %d", static_cast<int>(g_CsvRows.size()));
		}
		ImGui::End();
	}
}

NS_END

