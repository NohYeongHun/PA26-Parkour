#include "EnginePch.h"
#include "GUIManager.h"

#include "GameInstance.h"

CGUIManager::CGUIManager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }, m_pContext{ pContext },
	m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

void CGUIManager::Add_GUI_Func(function<void()> func)
{
	m_Functions.push_back(func);
}

void CGUIManager::Clear_Func()
{
	m_Functions.clear();
}

void CGUIManager::Use_Gizmo(CTransform* pTransform)
{
	if (m_pTransform == pTransform)
		return;

	Safe_Release(m_pTransform);
	m_pTransform = pTransform;
	Safe_AddRef(m_pTransform);
}

void CGUIManager::Use_Gizmo_Offset(_float3* pScale, _float3* pRotation, _float3* pTranslation)
{
	if (nullptr == pScale || nullptr == pRotation || nullptr == pTranslation)
		return;

	m_pScale = pScale;
	m_pRotation = pRotation;
	m_pTranslation = pTranslation;
}

void CGUIManager::Render_Gizmo(const _fmatrix& Matrix)
{
	_float4x4 ViewMatrix = {};
	_float4x4 ProjMatrix = {};
	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjMatrix, m_pGameInstance->Get_TransformState_Matrix(D3DTS::PROJ));

	_float4x4 WorldMatrix = {};
	XMStoreFloat4x4(&WorldMatrix, Matrix);

	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(0.f, 0.f, 1920.f, 1080.f);
	ImGuizmo::BeginFrame();
	ImGuizmo::PushID(100);
	ImGuizmo::Manipulate(
		reinterpret_cast<const _float*>(&ViewMatrix),
		reinterpret_cast<const _float*>(&ProjMatrix),
		m_CurrentGizmoOperation,
		m_CurrentGizmoMode,
		reinterpret_cast<_float*>(&WorldMatrix)
	);
	ImGuizmo::PopID();
}

HRESULT CGUIManager::Initialize(HWND hWnd)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	
	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pContext);

    return S_OK;
}

void CGUIManager::Update()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// GUI ?깅줉??Func ?섑뻾
	for (auto& Func : m_Functions)
		Func();

	Gizmo();
	Gizmo_Offset();

	// ImGui Render
	if (m_pGameInstance->Get_DIKeyState(DIK_END) == KEYSTATE::DOWN)
		m_isRender = !m_isRender;
}

void CGUIManager::Render()
{
	ImGui::Render();
	if(true == m_isRender)
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	
	m_pContext->OMGetRenderTargets(1, &m_pMainRTV, &m_pMainDSV);

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	m_pContext->OMSetRenderTargets(1, &m_pMainRTV, m_pMainDSV);

	Safe_Release(m_pMainRTV);
	Safe_Release(m_pMainDSV);
	m_pMainRTV = nullptr;
	m_pMainDSV = nullptr;
}

void CGUIManager::Gizmo()
{
	if (nullptr == m_pTransform)
		return;

	ImGui::Begin("Editor Transform");

	if (ImGuizmo::IsUsing())
	{
		ImGui::Text("Using gizmo");
		m_pTransform->Set_WorldMatrix(XMLoadFloat4x4(&m_ObjectWorldMatrix));
	}
	else
	{
		ImGui::Text(ImGuizmo::IsOver() ? "Over gizmo" : "");
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::TRANSLATE) ? "Over translate gizmo" : "");
		ImGui::SameLine();
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::ROTATE) ? "Over rotate gizmo" : "");
		ImGui::SameLine();
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::SCALE) ? "Over scale gizmo" : "");
	}

	ImGuizmo::SetDrawlist();

	if (m_pGameInstance->Get_DIKeyState(DIK_Q) == KEYSTATE::DOWN)
		m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (m_pGameInstance->Get_DIKeyState(DIK_R) == KEYSTATE::DOWN)
		m_CurrentGizmoOperation = ImGuizmo::SCALE;
	if (m_pGameInstance->Get_DIKeyState(DIK_W) == KEYSTATE::DOWN)
		m_CurrentGizmoOperation = ImGuizmo::ROTATE;

	_float3 vScale{}, vRotation{}, vTranslation{};
	XMStoreFloat4x4(&m_ObjectWorldMatrix, m_pTransform->Get_WorldMatrix());
	//XMStoreFloat4x4(&Matrix, XMMatrixTranspose(m_pTransform->Get_WorldMatrix()));

	// Scale, Rotation, Traslation 媛깆떊
	ImGuizmo::DecomposeMatrixToComponents(
		reinterpret_cast<const _float*>(&m_ObjectWorldMatrix),
		reinterpret_cast<_float*>(&vTranslation),
		reinterpret_cast<_float*>(&vRotation),
		reinterpret_cast<_float*>(&vScale)
	);

	ImGui::InputFloat3("Scale", reinterpret_cast<_float*>(&vScale));
	ImGui::InputFloat3("Rotation", reinterpret_cast<_float*>(&vRotation));
	ImGui::InputFloat3("Translation", reinterpret_cast<_float*>(&vTranslation));

	ImGuizmo::RecomposeMatrixFromComponents(
		reinterpret_cast<const _float*>(&vTranslation),
		reinterpret_cast<const _float*>(&vRotation),
		reinterpret_cast<const _float*>(&vScale),
		reinterpret_cast<_float*>(&m_ObjectWorldMatrix)
	);

	if (ImGuizmo::SCALE != m_CurrentGizmoMode)
	{
		if (ImGui::RadioButton("Local", m_CurrentGizmoMode == ImGuizmo::LOCAL))
			m_CurrentGizmoMode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton("World", m_CurrentGizmoMode == ImGuizmo::WORLD))
			m_CurrentGizmoMode = ImGuizmo::WORLD;
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_N) == KEYSTATE::DOWN)
		m_isSnap = !m_isSnap;

	//ImGui::Separator();
#pragma region Gizmo
	ImGuiIO io = ImGui::GetIO();
	ImGui::Text("X: %f Y: %f", io.MousePos.x, io.MousePos.y);
	//POINT ptMouse = m_pGameInstance->Get_MousePoint();
	//io.MousePos = ImVec2(ptMouse.x, ptMouse.y);
	if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::PRESS)
		io.MouseDown[0] = true;

	//ImGui::SetNextWindowPos(ImVec2(0, 0));
	//ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
	//ImGui::Begin("Gizmo", 0, ImGuiWindowFlags_NoInputs);


	_float4x4 ViewMatrix = {};
	_float4x4 ProjMatrix = {};
	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjMatrix, m_pGameInstance->Get_TransformState_Matrix(D3DTS::PROJ));
	//XMStoreFloat4x4(&ViewMatrix, XMMatrixTranspose(m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW)));
	//XMStoreFloat4x4(&ProjMatrix, XMMatrixTranspose(m_pGameInstance->Get_TransformState_Matrix(D3DTS::PROJ)));

#pragma endregion
	ImGui::End();

	ImGuizmo::SetRect(0.f, 0.f, io.DisplaySize.x, io.DisplaySize.y);
	//ImGuizmo::SetRect(0.f, 0.f, 1920.f, 1080.f);
	ImGuizmo::BeginFrame();
	ImGuizmo::Manipulate(
		reinterpret_cast<const _float*>(&ViewMatrix),
		reinterpret_cast<const _float*>(&ProjMatrix),
		m_CurrentGizmoOperation,
		m_CurrentGizmoMode,
		reinterpret_cast<_float*>(&m_ObjectWorldMatrix)
	);
}

void CGUIManager::Gizmo_Offset()
{
	if (nullptr == m_pScale || nullptr == m_pRotation || nullptr == m_pTranslation)
		return;

	ImGui::Begin("Editor Transform");

	_float3 vRotation = {};
	if (ImGuizmo::IsUsing())
	{
		ImGui::Text("Using gizmo");
		// Scale, Rotation, Traslation 媛깆떊

		ImGuizmo::DecomposeMatrixToComponents(
			reinterpret_cast<const _float*>(&m_ObjectWorldMatrix),
			reinterpret_cast<_float*>(m_pTranslation),
			reinterpret_cast<_float*>(&vRotation),
			reinterpret_cast<_float*>(m_pScale)
		);

		m_pRotation->x += vRotation.x;
		if (m_pRotation->x < -180.f)
			m_pRotation->x = 180.f;
		else if (m_pRotation->x > 180.f)
			m_pRotation->x = -180.f;

		m_pRotation->y += vRotation.y;
		if (m_pRotation->y < -180.f)
			m_pRotation->y = 180.f;
		else if (m_pRotation->y > 180.f)
			m_pRotation->y = -180.f;

		m_pRotation->z += vRotation.z;
		if (m_pRotation->z < -180.f)
			m_pRotation->z = 180.f;
		else if (m_pRotation->z > 180.f)
			m_pRotation->z = -180.f;
	}
	else
	{
		ImGui::Text(ImGuizmo::IsOver() ? "Over gizmo" : "");
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::TRANSLATE) ? "Over translate gizmo" : "");
		ImGui::SameLine();
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::ROTATE) ? "Over rotate gizmo" : "");
		ImGui::SameLine();
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::SCALE) ? "Over scale gizmo" : "");
	}

	ImGuizmo::SetDrawlist();

	if (m_pGameInstance->Get_DIKeyState(DIK_Q) == KEYSTATE::DOWN)
		m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (m_pGameInstance->Get_DIKeyState(DIK_R) == KEYSTATE::DOWN)
		m_CurrentGizmoOperation = ImGuizmo::SCALE;
	if (m_pGameInstance->Get_DIKeyState(DIK_W) == KEYSTATE::DOWN)
		m_CurrentGizmoOperation = ImGuizmo::ROTATE;

	ImGui::InputFloat3("Scale", reinterpret_cast<_float*>(m_pScale));
	ImGui::InputFloat3("Rotation", reinterpret_cast<_float*>(m_pRotation));
	ImGui::InputFloat3("Translation", reinterpret_cast<_float*>(m_pTranslation));

	vRotation = _float3(0.f, 0.f, 0.f);
	ImGuizmo::RecomposeMatrixFromComponents(
		reinterpret_cast<const _float*>(m_pTranslation),
		reinterpret_cast<const _float*>(&vRotation),
		reinterpret_cast<const _float*>(m_pScale),
		reinterpret_cast<_float*>(&m_ObjectWorldMatrix)
	);

	if (ImGuizmo::SCALE != m_CurrentGizmoMode)
	{
		if (ImGui::RadioButton("Local", m_CurrentGizmoMode == ImGuizmo::LOCAL))
			m_CurrentGizmoMode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton("World", m_CurrentGizmoMode == ImGuizmo::WORLD))
			m_CurrentGizmoMode = ImGuizmo::WORLD;
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_N) == KEYSTATE::DOWN)
		m_isSnap = !m_isSnap;

	//ImGui::Separator();
#pragma region Gizmo
	ImGuiIO io = ImGui::GetIO();
	ImGui::Text("X: %f Y: %f", io.MousePos.x, io.MousePos.y);
	//POINT ptMouse = m_pGameInstance->Get_MousePoint();
	//io.MousePos = ImVec2(ptMouse.x, ptMouse.y);
	if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::PRESS)
		io.MouseDown[0] = true;

	//ImGui::SetNextWindowPos(ImVec2(0, 0));
	//ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
	//ImGui::Begin("Gizmo", 0, ImGuiWindowFlags_NoInputs);


	_float4x4 ViewMatrix = {};
	_float4x4 ProjMatrix = {};
	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjMatrix, m_pGameInstance->Get_TransformState_Matrix(D3DTS::PROJ));
	//XMStoreFloat4x4(&ViewMatrix, XMMatrixTranspose(m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW)));
	//XMStoreFloat4x4(&ProjMatrix, XMMatrixTranspose(m_pGameInstance->Get_TransformState_Matrix(D3DTS::PROJ)));

#pragma endregion
	ImGui::End();

	ImGuizmo::SetRect(0.f, 0.f, io.DisplaySize.x, io.DisplaySize.y);
	//ImGuizmo::SetRect(0.f, 0.f, 1920.f, 1080.f);
	ImGuizmo::BeginFrame();
	ImGuizmo::Manipulate(
		reinterpret_cast<const _float*>(&ViewMatrix),
		reinterpret_cast<const _float*>(&ProjMatrix),
		m_CurrentGizmoOperation,
		m_CurrentGizmoMode,
		reinterpret_cast<_float*>(&m_ObjectWorldMatrix)
	);
}

CGUIManager* CGUIManager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd)
{
	CGUIManager* pInstance = new CGUIManager(pDevice, pContext);

	if (FAILED(pInstance->Initialize(hWnd)))
	{
		MSG_BOX("Failed to Create : GUIManager");
		Safe_Release(pInstance);
	}

    return pInstance;
}

void CGUIManager::Free()
{
	__super::Free();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// Gizmo Transform
	Safe_Release(m_pTransform);

	Safe_Release(m_pMainRTV);
	Safe_Release(m_pMainDSV);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
