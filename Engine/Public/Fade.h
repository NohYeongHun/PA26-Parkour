#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CFade final : public CBase
{
private:
	typedef struct tagFadeDesc {
		FADE					eFade = {};
		_float					fDuration = {};
		function<void()>	Func = { nullptr };
	}FADE_DESC;

private:
	explicit CFade(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CFade() = default;

public:
	void							OnFade(FADE eFade, _float fDuration, function<void()> func);

public:
	void							Initialize(_uint iWinSizeX, _uint iWinSizeY);

	void							Priority_Update(_float fTimeDelta);
	void							Update(_float fTimeDelta);

public:
	void							Render();

private:
	class CShader*				m_pShader = { nullptr };
	class CVIBuffer_Rect*		m_pVIBuffer = { nullptr };

	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	_float4x4						m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};

	FADE							m_eFade = { FADE::FADE_OUT };
	_float							m_fTimeAcc = {};
	_float							m_fDuration = { FLT_MIN };
	function<void()>			m_Func;
	_bool							m_isFade = { false };
	queue<FADE_DESC>		m_Fades;

private:
	void							Bind_Resource();

public:
	static		CFade*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual		void		Free() override;
};

NS_END