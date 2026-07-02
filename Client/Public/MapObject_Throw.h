#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CRigidbody;
class CModel_Streaming;
NS_END

NS_BEGIN(Client)
class CMapObject_Throw final : public CGameObject
{
public:
	typedef struct tagMapLoad
	{

		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		_float4x4* WorldMatrix = { nullptr };
		OBJECTTYPE eObjectType;
		_uint iLevel = {};
		_float3 vBoundingPos;
		_float3 vBoundingExtends;
	}MAP_LOAD;

private:
	CMapObject_Throw(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapObject_Throw(const CMapObject_Throw& Prototype);
	virtual ~CMapObject_Throw() = default;


public:
	virtual		HRESULT			Initialize_Prototype()override;
	virtual		HRESULT			Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;
	virtual		void			Render()override;
	virtual		void			Render_Shadow()override;

	void						OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
	void						OnCollider_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
private:
	void						Change_Level();
	void						Ready_Components(void* pArg);

	void						Collide();
	void						Graped();

	void						Calc_CombinedMatrix();
	void						Attach_Lerp();
	void						Attach_Pos();

private:
	class CModel_Streaming* m_pModelCom = {};
	CShader* m_pShaderCom = { nullptr };
	class CGameSystem* m_pGameSystem = { nullptr };
	CRigidbody* m_pDetectRigidbodyCom = { nullptr };
	CRigidbody* m_pThrowRigidbodyCom = { nullptr };
	CRigidbody* m_pCollideRigidbodyCom = { nullptr };

	_uint m_iShaderPassIndex = {};
	_float3 m_vTargetPos = {};
	_float m_fThrowTime = {};
	_float3 m_vStartPos = {};
	_float3 m_vImpulse = {};

	_bool m_IsGrabbed = { false };
	_bool m_IsThrow = { false };

	_float4 m_vOriginPos = {};

	CALLBACK_CLIENT m_Desc = {};
	_float m_fFlyTime = {};


	_float m_fattachTime = {};

	const _float4x4* m_pAttachBoneMatrix = { nullptr };
	const _float4x4* m_pAttachWorldMatrix = { nullptr };
	_float4x4 m_AttachMatrix = {};

 public:
	static CMapObject_Throw* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;
};

NS_END