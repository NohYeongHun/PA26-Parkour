#include "EnginePch.h"
#include "CollideComponent.h"

CCollideComponent::CCollideComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
{
}

CCollideComponent::CCollideComponent(const CCollideComponent& Prototype)
	: CComponent { Prototype}
{
}

void CCollideComponent::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if (nullptr == m_CallBack[ENUM_CLASS(COLLIDE_STATE::ENTER)])
		return;

	m_CallBack[ENUM_CLASS(COLLIDE_STATE::ENTER)](iLayer, pDesc, Manifold);
}

void CCollideComponent::OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if (nullptr == m_CallBack[ENUM_CLASS(COLLIDE_STATE::DURING)])
		return;

		m_CallBack[ENUM_CLASS(COLLIDE_STATE::DURING)](iLayer, pDesc, Manifold);
}

void CCollideComponent::OnCollide_Remove(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if (nullptr == m_CallBack[ENUM_CLASS(COLLIDE_STATE::REMOVE)])
		return;

	m_CallBack[ENUM_CLASS(COLLIDE_STATE::REMOVE)](iLayer, pDesc, Manifold);
}

void CCollideComponent::Free()
{
	__super::Free();
}
