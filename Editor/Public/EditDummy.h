#pragma once
#include "GameObject.h"

NS_BEGIN(Editor)

class CEditDummy abstract : public CGameObject
{
public:
	typedef struct tagDummyDesc : public CGameObject::GAMEOBJECT_DESC {
		_float3 vScale = _float3(1.f, 1.f, 1.f);
		_vector vPosition = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		_vector vRotation = XMQuaternionIdentity();
	}DUMMY_DESC;

protected:
	explicit CEditDummy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEditDummy(const CEditDummy& Prototype);
	virtual ~CEditDummy() = default;

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;
	virtual		void			Render_Shadow() override;

public:
	virtual	CGameObject*	Clone(void* pArg) PURE;
	virtual void			Free() override;
};

NS_END