#pragma once
#include "StaticObject.h"

NS_BEGIN(Engine)
class CModel_Streaming;
class CShader;
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CMapObject_Destruction_Debris final: public CStaticObject
{
public:
	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		const _float4x4* WorldMatrix = { nullptr };
		_uint iLevel = ENUM_CLASS(LEVEL::END);
		_float3 vImpulse = {};
	}MAP_LOAD;

	typedef struct tagResetDesc {
		_float3 vImpulse;
	}RESET_DESC;


private:
	CMapObject_Destruction_Debris(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapObject_Destruction_Debris(const CMapObject_Destruction_Debris& Prototype);
	virtual ~CMapObject_Destruction_Debris() = default;

public:
	virtual		HRESULT		Initialize_Prototype();
	virtual		HRESULT		Initialize_Clone(void* pArg);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	virtual		void			Late_Update(_float fTimeDelta);
	virtual		void			Render()override;
	virtual		void			Render_Shadow();

	HRESULT Ready_Component(void* pArg = nullptr);

	virtual void Bind_Resources();

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg)override;

private:
	_uint m_iShaderPassIndex = {};
	CModel_Streaming* m_pModelCom = { nullptr };
	CRigidbody* m_pRigidbodyCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	_bool m_IsTriggered = { false };
	_float3 m_vImpulse = {};
private:
	_float m_fTimeDelta = {};
	_float4x4 m_FixedPos = {};

public:
	static CMapObject_Destruction_Debris* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;
};

NS_END