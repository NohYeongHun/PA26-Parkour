#ifndef Engine_Define_h__
#define Engine_Define_h__

#pragma warning(disable: 4251)

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "FX11/d3dx11effect.h"
#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"
#include "DirectXTK/GeometricPrimitive.h"
#include "DirectXTK/SpriteBatch.h"
#include "DirectXTK/SpriteFont.h"
#include "DirectXTK/PrimitiveBatch.h"
#include "DirectXTK/Effects.h"
#include "DirectXTK/ScreenGrab.h"
#include "DirectXTK/DirectXTex.h"
using namespace DirectX;

// Comptr
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

// Fmod
#include "Fmod/fmod.hpp"
#define FMOD_CHANNEL_MAX 32



// Json
#include "Json/json.hpp"
using json = nlohmann::json;

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/ImGuiFileDialog.h"
#include "ImGui/ImGuiFileDialogConfig.h"
//#include "ImGui/ImApp.h"
#include "ImGui/ImGuizmo.h"
#include "ImGui/ImSequencer.h"
#include "ImGui/ImZoomSlider.h"
#include "ImGui/ImCurveEdit.h"
#include "ImGui/GraphEditor.h"

#pragma warning(push)
#pragma warning(disable: 26495)
// Jolt
#define JPH_NAMESPACE JPH
#include "Jolt/Jolt.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Body/BodyManager.h"
#include "Jolt/Physics/Body/BodyInterface.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Character/Character.h"
#include "Jolt/Physics/Character/CharacterVirtual.h"
#include "Jolt/Physics/Collision/CastResult.h"
#include "Jolt/Physics/Collision/RayCast.h"
#include "Jolt/Physics/Collision/NarrowPhaseQuery.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Core/JobSystemSingleThreaded.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Physics/Collision/CollisionDispatch.h"
using namespace JPH;
#pragma warning(pop)

// FreeType
#include "ft2build.h"
#include FT_FREETYPE_H

#include <vector>
#include <list>
#include <queue>
#include <map>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <string>
#include <unordered_map>
#include <ctime>
#include <fstream>
#include <Windows.h>
#include <fcntl.h>

#include <io.h>
#include <iostream>
using namespace std;

namespace Engine
{
	static const unsigned int g_iMaxNumBones = 512;

	//const unsigned int g_iMaxWidth = 16384;
	//const unsigned int g_iMaxHeight = 9216;	

	const unsigned int g_iMaxWidth = 4096;
	const unsigned int g_iMaxHeight = 2304;

	const unsigned int g_iMaxShadowMapSize = 8192;
	const unsigned int g_iSectorSize = 2048;

	const unsigned int g_iNumCascade = 4;
	//const float g_fLODDistance[4] = { 0.f, 1500.f, 3000.f, 4500.f };
	//const float g_fLODGap = { 1500.f };
	//const float g_fLODDistance[4] = { 0.f, 800.f, 1600.f, 2400.f };
	const float g_fLODGap = { 400.f };

	const unsigned int g_iMaxDecal = 128;

	const unsigned int g_iMaxSector = 16;

	const unsigned int g_iEnvMapSize = 512;

	const unsigned int g_iMaxEnvMap = 8;
}

#define MAX_RENDER_THREAD 5
#define MAX_DEPTH	5
#define MAX_MIPLEVEL 10

#include "Engine_Enum.h"
#include "Engine_Macro.h"
#include "Engine_Struct.h"
#include "Engine_Vertex.h"
#include "Engine_Typedef.h"
#include "Engine_Function.h"

using namespace Engine;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // Engine_Define_h__
