#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;
class CComputeShader;
class CShader;
class CTexture;

class CVolumetricFog final : public CBase
{
private:
	typedef struct tagVF_Data{
		_float4x4 PrevViewMatrix;	
		_float4x4 PrevProjMatrix;
		_float4x4 InvViewMatrix;
		_float4x4 InvProjMatrix;
		_float4 vCamPos;			// --
		_float3 vFogColor;
		_float fPadding123;			// --
		_float3 vFroxelSize;
		_float Padding;				//--
		_float fNear;				
		_float fFar;				
		_uint iSliceCount;			
		_uint iLightCount;			//--
		_float fCamNear;			
		_float fCamFar;				
		_float fWinSizeX;			
		_float fWinSizeY;			// --
		_float fLightIntensity;		
		_float fDensity;			
		_float fPhaseFunctionG;		
		_float fDensityScale;		// --
		_float fFogMaxHeight;		
		_float fFogMaxDistance;
		_float fHegihtFallOff;		
		_float fDistanceFallOff;	// --
		_float fGroundFallOff;		
		_float fNoiseScale;			
		_float fNoiseTimeDelta;
		_bool  IsTemporal;			// --
		_uint iRandCount;			
		_float fRayPhaseFunctionG;	
		_float fRayIntensity;
		_float fScatterWeight;		// --
		_float fFogBaseIntensity;
		_float fRayDensity;
		_float fRayDensityScale;
		_float fPadding2;
	}VF_DATA;

	enum class CS { VF_LIGHT, VF_BEER, VF_NOISE, END};
	enum class UAV { VF_LIGHT_FIRST, VF_LIGHT_SECOND, VF_BEER, VF_NOISE, END};
	enum class SRV { VF_LIGHT_FIRST, VF_LIGHT_SECOND, VF_BEER, VF_NOISE, LIGHT, END };
	enum class BUFFER { DATA, LIGHT, END};
	
private:
	CVolumetricFog(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CVolumetricFog() = default;

public:
	void						Set_FogFarRatioToCameraFar(_float fFogFarRatio);
	void						Set_FogRayIntensity(_float fRayIntensity) { m_VF_Data.fRayIntensity = fRayIntensity; }
	void						Set_FogMaxHeight(_float fFogMaxHeight) { m_VF_Data.fFogMaxHeight = fFogMaxHeight; }
	void						Set_FogMaxDistance(_float fFogMaxDistance) { m_VF_Data.fFogMaxDistance = fFogMaxDistance; }
	void						Set_FogDistanceFallOff(_float fDistanceFallOf) { m_VF_Data.fDistanceFallOff = fDistanceFallOf; }
	void						Set_FogRayDensityScale(_float fFogRayDensityScale) { m_VF_Data.fRayDensityScale = fFogRayDensityScale; }
	void						Set_FogScatterWeight(_float fFogScatterWeight) { m_VF_Data.fScatterWeight = fFogScatterWeight; }

public:
	HRESULT						Initialize(_uint iWinSizeX, _uint iWinSizeY);
	HRESULT						SetUp_FogNF();

	void						Begin_VF();
	void						Clear();
	void						Update_VF(_float fTimeDelta);

	HRESULT						Bind_VF_Resource(CShader* pShader, const _char* pTextureName, const _char* pFogRangeName);


//#ifdef _DEBUG
public:
	void						Setting_VF();
	_float TestScale = {  };
//#endif

private:
	_float						m_fFogFarRatioToCamera = {};

	_float3						m_vFroxelSize = {};
	_float2						m_vFogRange = {};
	_float						m_vFov = {};
	_float3						m_vDefinition = {};
	VF_DATA						m_VF_Data = {};

	_bool						m_IsUpdate = { false };
	_bool						m_IsFirst = { false };
	_float3						m_vNoiseSize = {};

	_uint						m_iMaxLight = {};
	vector<LIGHT_DATA>			m_LightDatas;

	CGameInstance*				m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	
	CComputeShader*				m_pCS[ENUM_CLASS(CS::END)] = { nullptr };

	ID3D11UnorderedAccessView*	m_pUAVs[ENUM_CLASS(UAV::END)] = { nullptr };

	ID3D11ShaderResourceView*	m_pSRVs[ENUM_CLASS(SRV::END)] = { nullptr };

	ID3D11Buffer*				m_pBuffers[ENUM_CLASS(BUFFER::END)] = {nullptr};

	_uint						m_iWriteIndex = {};
	_uint						m_iReadIndex = {};

	ID3D11SamplerState*			m_pDefaultSampler = { nullptr };
	ID3D11SamplerState*			m_pShadowSampler = { nullptr };

private:
	void						Update_Buffer(_float fTimeDelta);
	HRESULT						Ready_FroxelVolume();
	HRESULT						Ready_FogTexture(_uint iTextureIndex);
	HRESULT						Ready_NoiseTexture();
	HRESULT						Ready_Buffer();
	HRESULT						Ready_ComputeShader();
	HRESULT						Ready_Sampler();

	void						Make_NoiseTexture();

public:
	static CVolumetricFog*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, uint iWinSizeX, _uint iWinSizeY);
	virtual void				Free();
};

NS_END