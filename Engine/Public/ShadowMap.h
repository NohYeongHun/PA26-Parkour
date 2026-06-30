#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;
class CShader;
class CStaticObject;
class CComputeShader;

class CShadowMap final : public CBase
{


private:
	typedef struct ShadowMap_Data
	{
		_float4x4 SectorViewMatrix[64];
		_float4x4 SectorProjMatrix[64];
		_float4   vSectorUV[16];

		_int iNumSector;
		_int iNumSectorX;
		_int iNumSectorToLayer;
		_float Padding0;

		_float2 vSectorWorldSize;
		_float Padding1[2];

		_float2 vMin;
		_float Padding2[2];

		_float2 vMax;
		_float Padding3[2];


		_float2 vShadowMapSize;
		_float Padding4[2];
	}SHADOWMAP_DATA;

private:
	explicit CShadowMap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CShadowMap() = default;

public:
	const vector<BoundingBox*>& Get_ShadowMapSectors() { return m_Boundings; }

	const _uint					Get_ShadowMapLayer(_uint iSectorIndex) { return (iSectorIndex / m_iNumSectorToLayer); }
	ID3D11ShaderResourceView*	Get_DownSampleSRV() { return m_pDS_SRV; }
	ID3D11Buffer*				Get_DownSampleBuffer() { return m_pShadowMapBuffer; }
public:
	HRESULT						Setting_ShadowMap(const SHADOW_MAP_DESC& MapDesc);
	HRESULT						Bind_ShadowMap_Resources(CShader* pShader, const _char* pViewName, const _char* pProjName, _uint iSector);	// STATIC_OBJECT
	HRESULT						Bind_ShadowMap_Resources(CShader* pShader);																	// RENDERER

	HRESULT						Begin_ShadowMap();
	HRESULT						End_ShadowMap();

	HRESULT						DownSampleShadowMap();

	void						Clear();

	void						Render(CShader* pShader, class CVIBuffer_Rect* pVIBuffer);


private:
	vector<BoundingBox*>		m_Boundings;

	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	CGameInstance*				m_pGameInstance = { nullptr };

	SHADOW_MAP_DESC				m_MapDesc = {};

	ID3D11RenderTargetView*		m_pBackBuffer = { nullptr };
	ID3D11DepthStencilView*		m_pOriginalDSV = { nullptr };

	ID3D11DepthStencilView*		m_pShadowMapDSV = { nullptr };
	ID3D11ShaderResourceView*	m_pShadowMapSRV = { nullptr };
	
	_uint						m_iNumSector = {};
	_uint						m_iNumLayer = {};
	_uint						m_iNumSectorX_ToLayer = {};
	_uint						m_iNumSectorZ_ToLayer = {};
	_uint						m_iNumSectorToLayer = {};

	vector<_float4x4>			m_Matrices[ENUM_CLASS(D3DTS::END)];

	vector<_float4>				m_SectorUV;

	_uint						m_iShadowMapSizeX = {};
	_uint						m_iShadowMapSizeY = {};

	_float2						m_vShadowMapSize = {};
	_float2						m_vSectorSize = {};

	D3D11_VIEWPORT				m_PrevViewPort = {};
	_float						m_fMargin = { 0.1f };
	//DOWNSAMPLE
	ID3D11UnorderedAccessView*	m_pDS_UAV = { nullptr };
	ID3D11ShaderResourceView*	m_pDS_SRV= { nullptr };
	CComputeShader*				m_pCS = { nullptr };
	ID3D11Buffer*				m_pShadowMapBuffer = { nullptr };

private:
	void						Setting_ShadowMapViewPort(_uint iSector);
	void						Setting_PrevViewPort();

	HRESULT						Ready_ShadowMap();
	HRESULT						Ready_ShadowMapDownSample();
	HRESULT						Ready_SectorUV();
	HRESULT						Ready_Matrices();
	HRESULT						Ready_Buffer();

	_float3						Compute_CenterPos(_int iWeightX, _int iWeightZ, _float3 vOriginPos, _float3 vExtents);
	_float						Compute_MaxRadius(const BoundingBox* Bounding, _float3 vCenterPos);
	_float4x4					Make_ViewMatrix(_float3 vCenterPos, const BoundingBox* Bounding, _float3 vDir);
	_float4x4					Make_ProjMatrix(const BoundingBox* Bounding, _float4x4 ViewMatrix);


public:
	static CShadowMap*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void		Free() override;
};

NS_END