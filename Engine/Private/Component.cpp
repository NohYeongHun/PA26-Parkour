#include "EnginePch.h"
#include "Component.h"
#include "GameInstance.h"
#include "GameObject.h"

CComponent::CComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }, m_pContext { pContext },
	m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

CComponent::CComponent(const CComponent& Prototype)
	: m_pDevice { Prototype.m_pDevice }, m_pContext { Prototype.m_pContext },
	m_pGameInstance { CGameInstance::GetInstance() },
	m_isClone { true }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CComponent::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CComponent::Initialize_Clone(void* pArg)
{
	COMPONENT_DESC* pDesc = static_cast<COMPONENT_DESC*>(pArg);
	if (nullptr == pDesc)
		return S_OK;

	m_pOwner = pDesc->pOwner;

	return S_OK;
}

HRESULT CComponent::Render()
{
    return S_OK;
}

void CComponent::Free()
{
	__super::Free();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
