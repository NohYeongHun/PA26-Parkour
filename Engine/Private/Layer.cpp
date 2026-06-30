#include "EnginePch.h"
#include "Layer.h"

#include "GameObject.h"

CLayer::CLayer()
	: m_fTimeRate { 1.f }
{
}

HRESULT CLayer::Add_GameObject(CGameObject* pObject)
{
	if (nullptr == pObject)
		return E_FAIL;

	m_Objects.push_back(pObject);

	return S_OK;
}

CComponent* CLayer::Get_Component(_uint iGameObjectIndex, const _wstring& strComponentTag)
{
	if (m_Objects.size() <= iGameObjectIndex)
		return nullptr;

	return m_Objects[iGameObjectIndex]->Get_Component(strComponentTag);
}

void CLayer::Priority_Update(_float fTimeDelta)
{
	for (auto& pObject : m_Objects)
		if(true == pObject->IsActivate())
			pObject->Priority_Update(fTimeDelta * m_fTimeRate);
}

void CLayer::Update(_float fTimeDelta)
{
	if (0.f < m_fDuration)
		m_fDuration -= fTimeDelta;
	else if(false == m_isTimeStop)
		m_fTimeRate = 1.f;

	for (auto& pObject : m_Objects)
		if (true == pObject->IsActivate())
			pObject->Update(fTimeDelta * m_fTimeRate);
}

void CLayer::Late_Update(_float fTimeDelta)
{
	for (auto& pObject : m_Objects)
		if (true == pObject->IsActivate())
			pObject->Late_Update(fTimeDelta * m_fTimeRate);
}

CLayer* CLayer::Create()
{
	return new CLayer();
}

void CLayer::Free()
{
	__super::Free();

	for (auto& pObject : m_Objects)
		Safe_Release(pObject);
	m_Objects.clear();
}
