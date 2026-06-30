#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;

class CLight final : public CBase
{
private:
	explicit CLight();
	virtual ~CLight() = default;

public:
	_bool					IsInFrustrum();
	const LIGHT_DESC*		Get_LightDesc() { return &m_LightDesc; }
	HRESULT					Update_LightDesc(const LIGHT_DESC& LightDesc);
	void					Set_Active(_bool isActive) { m_isActive = isActive; }

#ifdef _DEBUG
	LIGHT_DESC*		Get_LightDesc_For_Map() { return &m_LightDesc; }
#endif
public:
	HRESULT			Initialize(const LIGHT_DESC& LightDesc);
	HRESULT			Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
	HRESULT			Render_EnvMap(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer, BoundingBox* pBounding);

private:
	CGameInstance*	m_pGameInstance = { nullptr };
	LIGHT_DESC		m_LightDesc = {};

	_bool			m_isActive = { true };

public:
	static		CLight*	Create(const LIGHT_DESC& LightDesc);
	virtual		void		Free() override;
};

NS_END