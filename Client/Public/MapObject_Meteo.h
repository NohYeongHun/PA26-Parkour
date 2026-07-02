#pragma once
#include "StaticObject.h"

NS_BEGIN(Engine)
class CModel_Streaming;
class CShader;
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CMapObject_Meteo final : public CGameObject
{
public:
	typedef struct tagMeteoDesc {
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		_float4 vSourPos = _float4(0.f, 0.f, 0.f, 1.f);
		_float4 vDestPos = _float4(0.f, 0.f, 0.f, 1.f);
		_float fDuration = {};
		_float4x4 WorldMatrix = {};
		_float fArchY = {}; 
		_uint TriggerIndex = {};
		_int TriggerActiveIndex;
		_uint iLevel = ENUM_CLASS(LEVEL::GAMEPLAY);
	}MAP_LOAD;

private:
	CMapObject_Meteo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapObject_Meteo(const CMapObject_Meteo& Prototype);
	virtual ~CMapObject_Meteo() = default;

public:
	virtual		HRESULT		Initialize_Prototype()override;
	virtual		HRESULT		Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;
	virtual		void			Render()override;
	void LerpPos(_float fTimeDelta);
private:
	void Ready_Components(void* pArg);

private:
	_float4 m_vSourPos = {};
	_float4 m_vDestPos = {};
	_float m_fFall = {};
	_float m_fDuration = {};

	_float m_farchY = {};
private:
	_uint m_iTriggerIndex = {};
	_int m_iTriggerActiveIndex = {};
	_uint m_iEffectFrame = {};
private:
	CShader* m_pShaderCom = { nullptr };

	//CRigidbody* m_pRigidbodyCom = { nullptr };
	class CGameSystem* m_pGameSystem = { nullptr };
	CModel_Streaming* m_pModelCom = { nullptr };
private:
	_uint m_iShaderPassIndex = {};
	_bool m_IsRender = { true };
	_bool m_IsTriggerd = { false };

	void* m_pTempPtr = { nullptr };
	void* m_pSecondTempPtr = { nullptr };
	void* m_pThirdTempPtr = { nullptr };
	_bool m_ISTrailEffect = { false };
	_bool m_IsSound = { false };

	_float3 m_vRadians = {};
	_uint m_iSoundChannel = {};
public:
	static vector<_wstring> m_SoundTags;
public:
	static CMapObject_Meteo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END