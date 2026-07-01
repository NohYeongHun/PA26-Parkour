#pragma once
#include "Base.h"
NS_BEGIN(Client)
class CMainApp : public CBase
{
private:
	explicit CMainApp();
	virtual ~CMainApp() = default;

public:
	HRESULT Initialize();
	void Post_Update();
	void Update(_float fTimeDelta);
	void Render();

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	class CGameSystem* m_pGameSystem = { nullptr };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

	_bool	m_isChangeLevel = { false };
	LEVEL	m_eNextLevel = { LEVEL::END };
	_bool	m_isLoad = false;

	ImGuiID m_iDockSpaceID = {};

	_float m_fTimeAcc = {};
	_uint m_iCnt = {};
	_uint m_iFrame = {};

private:
	void				Ready_Event();
	void				SetUp_CollisionLayer();
	void				Ready_Prototype_ForStatic();
	
	void				Start_Level();

public:
	static CMainApp* Create();
	virtual void Free() override;

};
NS_END

