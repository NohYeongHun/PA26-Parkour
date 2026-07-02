#pragma once
#include "StaticObject.h"
#include"Client_Enum.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CMapObject_Destruction final: public CStaticObject
{
public:
	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		_float4x4* WorldMatrix = { nullptr };
		_uint iLevel = ENUM_CLASS(LEVEL::END);
		_float3 vBoundingPos;
		_float3 vBoundingExtends;
		_float3 m_vImpulsePos = {};
		_float3 m_vImpulsePower = _float3(1.f, 1.f, 1.f);
		_uint iTriggerIndex = {};
	}MAP_LOAD;

private:
	CMapObject_Destruction(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapObject_Destruction(const CMapObject_Destruction& Prototype);
	virtual ~CMapObject_Destruction() = default;

public:
	virtual		HRESULT		Initialize_Prototype()override;
	virtual		HRESULT		Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;
	//virtual		void			Render(ID3D11DeviceContext* pDeferredContext, _uint iIndex) override;
	virtual		void			Render() override;
	
	virtual		void			Render_Shadow()override;

	HRESULT Ready_Component(void* pArg = nullptr);

	virtual void Bind_Resources();
private:
	_char m_ModelName[MAX_PATH] = {};
	_char m_BoneModelName[MAX_PATH] = {};
	_uint m_iShaderPassIndex = {};
	
	CRigidbody* m_pBoxRigidbodyCom = { nullptr };
	CRigidbody* m_pRigidbodyCom = { nullptr };
	CModel* m_pBoneModel = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	class CModel_Streaming* m_pModelCom;
	class CGameSystem* m_pGameSystem = { nullptr };

	_bool m_IsDestroy = { false };
	_bool m_IsChange = { false };
	_float3 m_vImpulsePos = {};
	_float3 m_vImpulsePower = {};
	
	_uint m_iTriggerIndex = {};
	_wstring m_szDebrisName;
	CALLBACK_CLIENT m_CallBack = {};
	void* m_pPullUI;
public:
	static vector<_wstring> m_SoundTags;
	
public:
	static CMapObject_Destruction* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;


};

NS_END