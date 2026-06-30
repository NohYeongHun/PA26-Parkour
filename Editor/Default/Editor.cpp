// Editor.cpp : ?좏뵆由ъ??댁뀡?????吏꾩엯?먯쓣 ?뺤쓽?⑸땲??
//

#include "EditorPch.h"
#include "framework.h"
#include "Editor.h"
#include "EditorApp.h"

#define MAX_LOADSTRING 100

// ?꾩뿭 蹂??
HWND			g_hWnd;
HINSTANCE		g_hInst;                                // ?꾩옱 ?몄뒪?댁뒪?낅땲??
WCHAR szTitle[MAX_LOADSTRING];                  // ?쒕ぉ ?쒖떆以??띿뒪?몄엯?덈떎.
WCHAR szWindowClass[MAX_LOADSTRING];            // 湲곕낯 李??대옒???대쫫?낅땲??

// ??肄붾뱶 紐⑤뱢???ы븿???⑥닔???좎뼵???꾨떖?⑸땲??
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// Console Create
	AllocConsole();

	FILE* pOut = { nullptr };
	FILE* pIn = { nullptr };
	freopen_s(&pOut, "CONOUT$", "w", stdout);
	freopen_s(&pOut, "CONOUT$", "w", stderr);
	freopen_s(&pIn, "CONIN$", "r", stdin);

	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	DWORD prev_mode;
	GetConsoleMode(hInput, &prev_mode);

	prev_mode &= ~ENABLE_QUICK_EDIT_MODE;
	prev_mode &= ~ENABLE_INSERT_MODE;
	prev_mode |= ENABLE_EXTENDED_FLAGS;

	SetConsoleMode(hInput, prev_mode);
#endif // _DEBUG

    // ?꾩뿭 臾몄옄?댁쓣 珥덇린?뷀빀?덈떎.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EDITOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // ?좏뵆由ъ??댁뀡 珥덇린?붾? ?섑뻾?⑸땲??
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EDITOR));

	MSG msg = {};
	msg.message = WM_NULL;

	CEditorApp* pEditorApp = CEditorApp::Create();
	if (nullptr == pEditorApp)
		return FALSE;
	CGameInstance* pGameInstance = CGameInstance::GetInstance();
	Safe_AddRef(pGameInstance);

	if (FAILED(pGameInstance->Add_Timer(TEXT("Timer_Default"))))
		return FALSE;
	if (FAILED(pGameInstance->Add_Timer(TEXT("Timer_60"))))
		return FALSE;

	_float fTimeAcc = { 0.f };

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		fTimeAcc += pGameInstance->Get_TimeDelta(TEXT("Timer_Default"));

		if (fTimeAcc >= 1.f / static_cast<const _float>(g_iFrame))
		{
			pEditorApp->Post_Update();
			pEditorApp->Update(pGameInstance->Get_TimeDelta(TEXT("Timer_60")));
			pEditorApp->Render();

			fTimeAcc = 0.f;
		}
	}

	Safe_Release(pEditorApp);
	Safe_Release(pGameInstance);

	return (int)msg.wParam;
}



//
//  ?⑥닔: MyRegisterClass()
//
//  ?⑸룄: 李??대옒?ㅻ? ?깅줉?⑸땲??
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EDITOR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ?⑥닔: InitInstance(HINSTANCE, int)
//
//   ?⑸룄: ?몄뒪?댁뒪 ?몃뱾????ν븯怨?二?李쎌쓣 留뚮벊?덈떎.
//
//   二쇱꽍:
//
//        ???⑥닔瑜??듯빐 ?몄뒪?댁뒪 ?몃뱾???꾩뿭 蹂?섏뿉 ??ν븯怨?
//        二??꾨줈洹몃옩 李쎌쓣 留뚮뱺 ?ㅼ쓬 ?쒖떆?⑸땲??
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   g_hInst = hInstance; // ?몄뒪?댁뒪 ?몃뱾???꾩뿭 蹂?섏뿉 ??ν빀?덈떎.

   RECT rc = { 0, 0, 1920, 1080 };
   //AdjustWindowRect(&rc, WS_POPUP, FALSE);
   AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, 
	   WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 
	   rc.right - rc.left, rc.bottom - rc.top,
	   nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hWnd = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ?⑥닔: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ?⑸룄: 二?李쎌쓽 硫붿떆吏瑜?泥섎━?⑸땲??
//
//  WM_COMMAND  - ?좏뵆由ъ??댁뀡 硫붾돱瑜?泥섎━?⑸땲??
//  WM_PAINT    - 二?李쎌쓣 洹몃┰?덈떎.
//  WM_DESTROY  - 醫낅즺 硫붿떆吏瑜?寃뚯떆?섍퀬 諛섑솚?⑸땲??
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// ImGui Key / Mouse ?낅젰??諛쏄린?꾪븳 ?몃뱾??
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;


	switch (message)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ?뺣낫 ????곸옄??硫붿떆吏 泥섎━湲곗엯?덈떎.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
