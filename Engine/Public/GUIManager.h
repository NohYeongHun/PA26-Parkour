#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGUIManager final : public CBase
{
private:
	explicit CGUIManager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CGUIManager() = default;

public:
	ImGuiContext*		Get_ImGuiContext() { return ImGui::GetCurrentContext(); }
	void					Add_GUI_Func(function<void()> func);
	void					Clear_Func();

	// Gizmo ?곸슜??媛앹껜??Transform ?꾨떖
	void					Use_Gizmo(class CTransform* pTransform = nullptr);
	void					Use_Gizmo_Offset(_float3* pScale, _float3* pRotation, _float3* pTranslation);

	// Gizmo Render??
	void					Render_Gizmo(const _fmatrix& Matrix);

public:
	HRESULT				Initialize(HWND hWnd);
	void					Update();
	void					Render();

private:
	class CGameInstance*		m_pGameInstance = { nullptr };
	ID3D11Device*					m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	ID3D11RenderTargetView*	m_pMainRTV = { nullptr };
	ID3D11DepthStencilView*	m_pMainDSV = { nullptr };

	vector<function<void()>>	m_Functions;

	// Gizmo Offset
	_float3*							m_pScale = { nullptr };
	_float3*							m_pRotation = { nullptr };
	_float3*							m_pTranslation = { nullptr };

	// Gizmo ?ъ슜?????꾩슂??Transform
	class CTransform*				m_pTransform = { nullptr };
	_float4x4							m_ObjectWorldMatrix = {};
	// Gizmo Setting
	ImGuizmo::OPERATION		m_CurrentGizmoOperation = { ImGuizmo::TRANSLATE };
	ImGuizmo::MODE				m_CurrentGizmoMode = { ImGuizmo::WORLD };
	// Gizmo Snap
	_bool								m_isSnap = { false };

	_bool								m_isRender = { true };

private:
	void					Gizmo();
	void					Gizmo_Offset();

public:
	static		CGUIManager*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd);
	virtual		void				Free() override;
};

NS_END