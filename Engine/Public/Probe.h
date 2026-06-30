#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;
class CGameObject;
class CStaticObject;
class CShader;
class CVIBuffer_Rect;

class CProbe final : public CBase
{
private:
	typedef struct tagFace
	{
		_float3 vDir;
		_float3 vUp;
	}FACE;

private:
	CProbe(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CProbe() = default;

public:
	HRESULT						Initialize(const _float3& vCenter, const _float& fRange);
	_bool						IsInProbe(BoundingBox* pObjectBounding);
	_bool						IsInFrustrum();
	
	void						Fill_Data(ENV_MAP* pOut);

	ID3D11ShaderResourceView*	Get_EnvMap() { return m_pSRV; }
	void						Render(CShader* pShader, CVIBuffer_Rect* pVIBuffer_Rect, _uint iIndex);
	void						Add_SkyBox(CGameObject* pSkyBox);
	void						Add_StaticObject(CStaticObject* pStaticObject);

private:
	BoundingBox*				m_pBounding = { nullptr };

	vector<CGameObject*>		m_SkyBoxs;
	vector<CStaticObject*>		m_StaticObjects;				
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	CGameInstance*				m_pGameInstance = { nullptr };

	_float4						m_vCenter = {};
	_float						m_fRange = {};
	vector<_float4x4>			m_Matrices[ENUM_CLASS(D3DTS::END)];
	vector<_float4x4>			m_MatricesInv[ENUM_CLASS(D3DTS::END)];

	ID3D11Texture2D*			m_pTexture = { nullptr };
	ID3D11RenderTargetView*		m_pRTVs[6] = {nullptr};
	ID3D11ShaderResourceView*	m_pSRV = { nullptr };

	ID3D11DepthStencilView*		m_pDSV = { nullptr };

	ID3D11RenderTargetView*		m_pBackBuffer = { nullptr };
	ID3D11DepthStencilView*		m_pOriginDSV = { nullptr };

	D3D11_VIEWPORT				m_PrevViewPort = {};

	mutex						m_AddMutex;

private:
	void					Render_SkyBoxs(_float4x4 ViewMatrix, _float4x4 ProjMatrix);
	void					Render_StaticObjects(_float4x4 ViewMatrix, _float4x4 ProjMatrix);
	HRESULT					Ready_Matrices();
	HRESULT					Ready_Resources();

	void					Setting_ProbeViewPort();
	void					Setting_PrevViewPort();
	HRESULT					Bind_ShaderResource(CShader* pShader, _uint iCubeFace);

public:
	static CProbe*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _float3& vCenter, const _float& fRange);
	virtual void			Free() override;
};

NS_END