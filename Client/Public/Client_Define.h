#pragma once

#include "../Default/framework.h"
#include <process.h>

#include "Client_Struct.h"
#define GRAVITY 98.f

namespace Client
{
	// Window SIze
	const unsigned int		g_iWinSizeX = 1920;
	const unsigned int		g_iWinSizeY = 1080;
	const unsigned int		g_iFrame = 250;
}

extern HWND			g_hWnd;
extern HINSTANCE	g_hInst;
using namespace Client;