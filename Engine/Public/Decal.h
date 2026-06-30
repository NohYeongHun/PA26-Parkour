#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CTexture;
class CVIBuffer_Decal;
class CShader;
class CGameInstance;

class ENGINE_DLL CDecal final : public CBase
{
private:
	typedef struct tagDecalUpdateData
	{
		_float fBlendTime;
		_float fLifeTime;

		_bool IsEqual;
		_float fCurrentTime;

		_vector vStartScale;
		_vector vStartRotation;
		_vector vStartPosition;

		_vector vEndScale;
		_vector vEndRotation;
		_vector vEndPosition;
	}DECAL_UPDATE_DATA;


private:
	typedef pair<DECAL_UPDATE_DATA, VTXINSTANCE_DECAL> DECAL_INSTANCE_DATA;
	typedef pair<DECAL_DATA::TYPE, DECAL_INSTANCE_DATA> DECAL_INSTANCE;

private:
	CDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CDecal() = default;

public:
	HRESULT						Initialize(const _tchar* pFilePath[ENUM_CLASS(TEXTURETYPE::END)], const _float3& vEmissiveLuminance);
	void						Update(_float fTimeDelta);
	void						Render(CShader* pShader);
	
	HRESULT						Add_DecalData(const DECAL_DATA& Decal);
	ID3D11ShaderResourceView*	Get_DecalSRV(TEXTURETYPE eTextureType);
	
private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	CGameInstance*				m_pGameInstance = { nullptr };

	list<DECAL_INSTANCE>		m_DecalDatas;
	
	_uint						m_iNumDecals = {};

	CTexture*					m_pDecalTexture[ENUM_CLASS(TEXTURETYPE::END)] = { nullptr };
	_float3						m_vEmiisiveLuminance = {};
	CVIBuffer_Decal*			m_pVIBuffer_Decal = { nullptr };

private:
	HRESULT						Add_DecalTexture(const _tchar* pFilePath[ENUM_CLASS(TEXTURETYPE::END)], _float3 vEmissiveLuminance);
	_bool						Update_InstanceData(DECAL_INSTANCE_DATA& Data, _float fTimeDelta);
	HRESULT						Bind_Resources(CShader* pShader);

public:
	static CDecal*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath[ENUM_CLASS(TEXTURETYPE::END)],
								const _float3& vEmissiveLuminance);
	virtual void				Free() override;
};

NS_END