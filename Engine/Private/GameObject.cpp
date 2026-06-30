#include "EnginePch.h"
#include "GameObject.h"
#include "GameInstance.h"

#include "Transform.h"

CGameObject::CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice {pDevice}, m_pContext {pContext},
	m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

CGameObject::CGameObject(const CGameObject& Prototype)
	: m_pDevice { Prototype.m_pDevice }, m_pContext { Prototype.m_pContext },
	m_pGameInstance { CGameInstance::GetInstance() },
	m_isActivate { Prototype.m_isActivate }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

CComponent* CGameObject::Get_Component(const _wstring& strComponentTag)
{
	auto iter = m_Components.find(strComponentTag);
	if (iter == m_Components.end())
		return nullptr;

	return iter->second;
}

HRESULT CGameObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGameObject::Initialize_Clone(void* pArg)
{
	m_pTransformCom = CTransform::Create(m_pDevice, m_pContext);
	if (FAILED(m_pTransformCom->Initialize_Clone(pArg)))
		return E_FAIL;

	m_Components.emplace(TEXT("Com_Transform"), m_pTransformCom);
	Safe_AddRef(m_pTransformCom);

	return S_OK;
}

void CGameObject::Priority_Update(_float fTimeDelta)
{
}

void CGameObject::Update(_float fTimeDelta)
{
}

void CGameObject::Late_Update(_float fTimeDelta)
{
}

void CGameObject::Render()
{
}

void CGameObject::Render_Shadow()
{
}

void CGameObject::Render_OutLine()
{
}

void CGameObject::Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix)
{
}

HRESULT CGameObject::Add_Component(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg)
{
	auto iter = m_Components.find(strComponentTag);
	if (iter != m_Components.end())
		return E_FAIL;

	CBase* pClone = m_pGameInstance->Clone_Prototype(iPrototypeLevelID, strPrototypeTag, PROTOTYPE::COMPONENT, pArg);

	ASSERT_CRASH(pClone);

	CComponent* pComponent = static_cast<CComponent*>(pClone);
	m_Components.emplace(strComponentTag, pComponent);
	*ppOut = pComponent;
	Safe_AddRef(pComponent);

	return S_OK;
}

void CGameObject::Free()
{
	__super::Free();

	for (auto& Pair : m_Components)
		Safe_Release(Pair.second);
	m_Components.clear();
	
	Safe_Release(m_pTransformCom);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
