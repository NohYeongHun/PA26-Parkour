#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;
class CProbe;
class CGameObject;
class CStaticObject;
class CShader;
class CVIBuffer_Rect;

class CEnvironmentMap final : public CBase
{
private:
	CEnvironmentMap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CEnvironmentMap() = default;

public:
	HRESULT						Initialize();
	HRESULT						Add_Probe(const _float3& vCenter, const _float& fRange);
	void						Bake_EnvMaps();

	void						Add_EnvMap_SkyBox(CGameObject* pSkyBox);
	void						Add_EnvMap_StaticObject(CStaticObject* pStaticObject);

	HRESULT						Bind_EnvMapDatas(CShader* pShader, const _char* pTextureName, const _char* pBufferName, const _char* pHasEnvMapName, const _char* pNumEnvMapName);

	void						Clear();

private:
	vector<CProbe*>				m_Probes;
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	CGameInstance*				m_pGameInstance = { nullptr };

	CShader*					m_pShader = { nullptr };
	CVIBuffer_Rect*				m_pVIBuffer_Rect = { nullptr };

	// Deferred Matrix
	_float4x4					m_WorldMatrix = {};
	_float4x4					m_ViewMatrix = {};
	_float4x4					m_ProjMatrix = {};

	_uint						m_iIndex = {}; // Save Index

	_uint						m_iMaxBindEnvMap = {};
	vector<ENV_MAP>				m_EnvMapDatas;


	ID3D11Buffer*				m_pProbeBuffer = { nullptr };
	ID3D11ShaderResourceView*	m_pStructureSRV = {nullptr};

private:
	HRESULT						Ready_Components();
	HRESULT						Ready_Buffer();
	HRESULT						Ready_MRT();
	HRESULT						Ready_RT();

public:
	static CEnvironmentMap* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void			Free() override;
};

NS_END