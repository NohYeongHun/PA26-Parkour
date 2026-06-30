#pragma once

#include "../Default/framework.h"
#include <process.h>

#include "Editor_Enum.h"
#include "Editor_Struct.h"

#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"

#define GRAVITY 98.f

namespace Editor
{
	// Window SIze
	const unsigned int		g_iWinSizeX = 1920;
	const unsigned int		g_iWinSizeY = 1080;
	const unsigned int		g_iFrame = 250;
}

extern HWND			g_hWnd;
extern HINSTANCE		g_hInst;
using namespace Editor;