#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CModel_Streaming;
class CShader;
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CMapObject_Collaps : public CGameObject
	//논스태틱을 쓰자.
{
public:
	typedef struct tagMeteoDesc {
		_char ModelName[MAX_PATH] = {};
		_uint iLevel = {};
		_uint iShaderPassIndex = {};
		_float4x4 vSourWorldMatrix = {};
		_float4x4 vDestWorldMatrix = {};
		_float fDuration = {};
		_uint TriggerIndex = {};
		_int TriggerActiveIndex = { -1 };
	}MAP_LOAD;

	union MyFloat4 {
		_vector Vec;
		_float4 float4;
		_float arr[4];
	};

private:
	CMapObject_Collaps(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapObject_Collaps(const CMapObject_Collaps& Prototype);
	virtual ~CMapObject_Collaps() = default;

public:
	virtual		HRESULT		Initialize_Prototype()override;
	virtual		HRESULT		Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;
	virtual		void			Render()override;
	void LerpPos(_float fTimeDelta);

public:
	void OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
private:
	void Ready_Components(void* pArg);

private:
	_float4x4 m_SourMat = {};
	_float4x4 m_DestMat = {};
	_float m_fFall = {};
	_float m_fDuration = {};

	MyFloat4 m_vSourScale, m_vSourRot, m_vSourTrans;
	MyFloat4 m_vDestScale, m_vDestRot, m_vDestTrans;

private:
	_uint m_iTriggerIndex = {};

private:
	CShader* m_pShaderCom = { nullptr };

	CRigidbody* m_pSourRigidbodyCom = { nullptr };
	CRigidbody* m_pDestRigidbodyCom = { nullptr };
	CRigidbody* m_pBoxRigidbodyCom = { nullptr };
	class CGameSystem* m_pGameSystem = { nullptr };
	CModel_Streaming* m_pModelCom = { nullptr };

private:
	class CTransform* m_pTargetTransform = { nullptr };
	_uint m_iShaderPassIndex = {};
	_bool m_IsRender = { true };
	_bool m_IsTriggerd = { false };
	CALLBACK_CLIENT m_CallBack = {};
	Mutex m_Mutex;
	_float4x4 m_SmokePoint;
	_float4x4 m_SmokePoint2;
	void* m_pPullUI = { nullptr };

	_bool m_IsSound = { false };
public:
	static CMapObject_Collaps* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END