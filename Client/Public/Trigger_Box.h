#pragma once
#include "GameObject.h"
#include"Client_Enum.h"

NS_BEGIN(Engine)
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CTrigger_Box final : public CGameObject
{
public:
	using TriggerCallback = function<void(void*)>;
	typedef struct tagTriggerBox
	{
		_float4x4* WorldMatrix = { nullptr };
		_float3 vExtends;
		_uint iLevel = ENUM_CLASS(LEVEL::END);
		OBJECTTYPE eObjectType;
		_uint iTriggerIndex;
	}TRIGGER;

	/*
	make_pair(TEXT("Action_Asphodel_Barrens_Horizon"), make_pair(*m_pTransformCom->Get_WorldMatrixPtr(), true
	*/

	typedef struct tagCamInfo {
		_wstring szCamTag;
		_float4x4 CamMatrix;
		_bool IsMaintain;
		_bool isEscape = false;
	}CAM_INFO;
private:
	CTrigger_Box(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTrigger_Box(const CTrigger_Box& Prototype);
	virtual ~CTrigger_Box() = default;

public:
	virtual		HRESULT		Initialize_Prototype()override;
	virtual		HRESULT		Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;


private:
	void Ready_Components(void* pArg);
	void Collision_Enter();
	void Collision_During();
	void Collision_End();
	void Register_Trigger();

	void UI_Set(_bool B);
private:
	CRigidbody* m_pRigidbodyCom = { nullptr };
	class CGameSystem* m_pGameSystem = { nullptr };

private:
	vector< TriggerCallback> m_Functions;
	_uint m_iTriggerIndex = {};
	_bool m_IsTriggered = { false };
	_bool   m_bOnCoolDown = { false }; 
	_float  m_fCoolDown = { 0.f };     

	void* m_pTempPtr = { nullptr };
	void* m_pSecondTempPtr = { nullptr };
	CAM_INFO* m_CamMatrix;
	_bool m_IsDoingPalette = { false };

	_uint m_iMiniGameClearNum = {};
public:
	static CTrigger_Box* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END