#pragma once
#include "Base.h"

NS_BEGIN(Editor)

class CEditorApp final : public CBase
{
private:
	explicit CEditorApp();
	virtual ~CEditorApp() = default;

public:
	HRESULT			Initialize();
	void				Post_Update();						// ?덈꺼 ?꾪솚
	void				Update(_float fTimeDelta);
	void				Render();

private:
	class CGameInstance*	m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	_bool							m_isChangeLevel = { false };
	LEVEL							m_eNextLevel = { LEVEL::END };
	_bool							m_isLoad = { false };

	// Frame
	_uint							m_iFrame = {};
	_float							m_fTimeAcc = {};
	_uint							m_iCnt = {};

private:
	void				SetUp_CollisionLayer();
	void				Ready_Event();
	void				Ready_Prototype_ForStatic();
	void				Ready_Dummies();
	void				Ready_Sound();
	void				Start_Level();

public:
	static		CEditorApp* Create();
	virtual		void			Free() override;
};

NS_END