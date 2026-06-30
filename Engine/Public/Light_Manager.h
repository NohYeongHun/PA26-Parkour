#pragma once
#include "Base.h"

#include "Light.h"

NS_BEGIN(Engine)

class CLight_Manager final : public CBase
{
private:
	explicit CLight_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLight_Manager() = default;

public:
	HRESULT				Initialize();

	const LIGHT_DESC*	Get_LightDesc(const _wstring& strLightTag);
	HRESULT				Update_LightDesc(const _wstring& strLightTag, const LIGHT_DESC& LightDesc);
	void				Set_Active(const _wstring& strLightTag, _bool isActive);
#ifdef _DEBUG
	LIGHT_DESC*			Get_LightDesc_For_Map(const _wstring& strLightTag);
#endif

	const vector<LIGHT_DATA>* Get_LightDatas() { return &m_LightDatas; }

public:
	void					Update_Light();
	HRESULT					Add_Light(const _wstring& strLightTag, const LIGHT_DESC& LightDesc);
	HRESULT					Clear_Light();
	HRESULT					Bind_LightDatas(class CShader* pShader);
	HRESULT					Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
	HRESULT					Render_EnvMap(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer, BoundingBox* pBounding);

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	map<_wstring, CLight*>		m_Lights;
	LIGHT_DESC*					m_pShadowLight;

	_uint						m_iMaxLights = {};
	_uint						m_iNumLights = {};
	vector<LIGHT_DATA>			m_LightDatas;

	ID3D11ShaderResourceView*	m_pLightSRV = {nullptr};
	ID3D11Buffer*				m_pLightBuffer = { nullptr };

private:
	HRESULT						Ready_LightBuffer();
	void						Update_Buffer();

public:
	static		CLight_Manager*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END