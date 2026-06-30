#pragma once
#include "GameObject.h"
#include"Editor_Enum.h"

NS_BEGIN(Engine)
class CRigidbody;
NS_END

NS_BEGIN(Editor)
class CEdit_TriggerBox final: public CGameObject
{
public:
	using TriggerCallback = function<void(void*)>;
	typedef struct tagTriggerBox
	{
		_float4x4* WorldMatrix = { nullptr };
		_float3 vExtends;
		_uint iLevel = ENUM_CLASS(LEVEL::MAP);
		OBJECTTYPE eObjectType;
		_int iTriggerIndex = {-1};
	}TRIGGER;

private:
	CEdit_TriggerBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_TriggerBox(const CEdit_TriggerBox& Prototype);
	virtual ~CEdit_TriggerBox() = default;

public:
	virtual		HRESULT		Initialize_Prototype()override;
	virtual		HRESULT		Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;


	virtual void Set_ImGuiOption();
private:
	void Ready_Components(void* pArg);

private:
	CRigidbody* m_pRigidbodyCom = { nullptr };
	class CMap_Interface* m_pMapInterface = { nullptr };

	void Collision();

private:
	_uint m_iTriggerIndex = {};
	_float3 m_vExtends = {};
public:
	static _uint iTriggerIndex;
public:
	static CEdit_TriggerBox* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END