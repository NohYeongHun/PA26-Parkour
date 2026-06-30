#include "EnginePch.h"
#include "ContainerObject.h"

#include "GameInstance.h"

#include "PartObject.h"

CContainerObject::CContainerObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject { pDevice, pContext }
{
}

CContainerObject::CContainerObject(const CContainerObject& Prototype)
    : CGameObject { Prototype }
{
}

HRESULT CContainerObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CContainerObject::Initialize_Clone(void* pArg)
{
    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    return S_OK;
}

void CContainerObject::Priority_Update(_float fTimeDelta)
{
}

void CContainerObject::Update(_float fTimeDelta)
{
}

void CContainerObject::Late_Update(_float fTimeDelta)
{
}

void CContainerObject::Render()
{
}

HRESULT CContainerObject::Add_PartObject(const _wstring& strPartObjectName, _uint iPrototypeLevelID, const _wstring& strPrototypeTag, void* pArg)
{
    if (nullptr != Find_PartObject(strPartObjectName))
        return E_FAIL;

    CPartObject* pPartObject = dynamic_cast<CPartObject*>(m_pGameInstance->Clone_Prototype(iPrototypeLevelID, strPrototypeTag, PROTOTYPE::GAMEOBJECT, pArg));
    if (nullptr == pPartObject)
        return E_FAIL;

    m_PartObjects.emplace(strPartObjectName, pPartObject);

    return S_OK;
}

CPartObject* CContainerObject::Find_PartObject(const _wstring& strPartObjectName)
{
    auto iter = m_PartObjects.find(strPartObjectName);
    if (iter == m_PartObjects.end())
        return nullptr;

    return iter->second;
}

void CContainerObject::Free()
{
    __super::Free();

    for (auto& Pair : m_PartObjects)
        Safe_Release(Pair.second);
    m_PartObjects.clear();
}
