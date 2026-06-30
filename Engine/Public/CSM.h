#pragma once
#include "Base.h"

NS_BEGIN( Engine )

class CGameInstance;
class CShader;

class CCSM final : public CBase
{
private:
	explicit CCSM(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CCSM() = default;

public:
	HRESULT					SetUp_ShadowLight(const _wstring& strLightTag);
	HRESULT					SetUp_ShadowNF();
public:
	HRESULT					Initialize();
	void					Update_CSM();
	void					Clear();

	HRESULT					Bind_CSM_Resources(CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pLightDirName);
	HRESULT					Bind_ShadowDistance_Resource(CShader* pShader, const _char* pDistanceName, const _char* pLastDistanceName);

	HRESULT					Bind_CSM_SRV(CShader* pShader, const _char* pConstantName);
	HRESULT					Begin_CSM();
	HRESULT					End_CSM();

	void					Render(CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	CGameInstance*				m_pGameInstance = { nullptr };

	const LIGHT_DESC*			m_pLightDesc = { nullptr };
	
	ID3D11DepthStencilView*		m_pShadowDSV = { nullptr };
	ID3D11ShaderResourceView*	m_pShadowSRV = { nullptr };

	ID3D11RenderTargetView*		m_pBackBuffer = { nullptr };
	ID3D11DepthStencilView*		m_pOriginalDSV = { nullptr };

	_uint						m_iNumClip = {};
	_uint						m_iNumClipDistance = {};

	_float4x4					m_Matrices[ENUM_CLASS( D3DTS::END )][g_iNumCascade];
	_float						m_fClipZ[g_iNumCascade];
	_float						m_fClipDistance[5];

	_float						m_fCameraNear = {};
	_float						m_fCutFar = {};
	_float						m_fCameraFar = {};

private:
	HRESULT						Ready_CSM_View();

	void						Update_Matrices();
	void						Make_Matrices(const _float4* pFrustrumPoints);

	_vector						Compute_Center(const _float4* pFrustrumPoints);
	_float						Compute_Radius(const _float4* pFrustrumPoints, _vector vCenterPos );
	_float						Compute_ClipDistance(_float fNear, _float fFar, _uint iIndex, _uint iNumClip, _float fLambda);

	_matrix						Make_SplitViewMatrix(const _float4* pFrustrumPoints);
	_matrix						Make_SplitProjMatrix(const _float4* pFrustrumPoints, _fmatrix ShadowViewMatrix);
//	void						Make_ClipZ();

public:
	static CCSM*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void				Free() override;
};

NS_END