#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CSlide_Navigation final: public CGameObject
{
public:
	typedef struct tagNavigationDesc {
		_bool IsStart;
		_float3 vExtends;
		_float4x4 WorldMat;
		_uint iPathSize;
		_float4* pPath;
	}SLIDE_NAVIGATION_DESC;
private:
	explicit CSlide_Navigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CSlide_Navigation(const CSlide_Navigation& Prototype);
	virtual ~CSlide_Navigation() = default;

public:
	virtual		HRESULT			Initialize_Prototype()override;
	virtual		HRESULT			Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;
	virtual		void			Render()override;

private:
	void						Ready_Component(void* pArg);

private:
	CRigidbody* m_pRigidbodyCom = { nullptr };
	CALLBACK_CLIENT m_CallBack = {};
public:
	static CSlide_Navigation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;
};

NS_END