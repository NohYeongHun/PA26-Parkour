#pragma once
#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCollideComponent abstract : public CComponent
{
	typedef function<void(_uint, void*, const ContactManifold&)> CALL_BACK_FUNC;
protected:
	explicit CCollideComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CCollideComponent(const CCollideComponent& Prototype);
	virtual ~CCollideComponent() = default;

public:
	void						SetUp_CallBack(COLLIDE_STATE eState, CALL_BACK_FUNC func) { m_CallBack[ENUM_CLASS(eState)] = func; }

public:
	virtual HRESULT		Initialize_Prototype() { return S_OK; }
	virtual HRESULT		Initialize_Clone(void* pArg) { return S_OK; }
	virtual HRESULT		Render() { return S_OK; }

	virtual void				OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
	virtual void				OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
	virtual void				OnCollide_Remove(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

private:
	CALL_BACK_FUNC		m_CallBack[ENUM_CLASS(COLLIDE_STATE::END)];

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

NS_END