#include "Editorpch.h"
#include "Effect_Decal.h"

CEffect_Decal::CEffect_Decal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CEffect_Decal::CEffect_Decal(const CEffect_Decal& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CEffect_Decal::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Decal::Initialize_Clone(void* pArg)
{
	DECAL_DESC* pDesc = static_cast<DECAL_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    m_vColor = pDesc->vColor;
    m_LifeTime = pDesc->LifeTime;
	m_wstrMyTag = pDesc->wstrDecalTag;
	m_fBlendTime = pDesc->fBlendTime;
	m_fEmissiveIntensity = pDesc->fEmissiveIntensity;

   // m_pTransformCom->Scale(_float3(pDesc->vSize.x, pDesc->vSize.y, pDesc->vSize.z));

    //처음 만들어질 땐 무조건 활성화 ?
    m_isActivate = false;

    return S_OK;
}

void CEffect_Decal::Priority_Update(_float fTimeDelta)
{
}

void CEffect_Decal::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	DECAL_DATA Desc{};
	Desc.eType = DECAL_DATA::NONSTATIC;
	Desc.fLifeTime = m_LifeTime;
	Desc.vColor = m_vColor;
	Desc.WorldMatrix = m_ComBindMatrix; /*m_pTransformCom->Get_WorldMatrix();*/
	Desc.EndWorldMatrix = m_ComBindMatrix;
	Desc.fBlendTime = m_fBlendTime;
	Desc.fEmissiveIntensity = m_fEmissiveIntensity;

	m_pGameInstance->Add_DecalData(m_wstrMyTag, Desc);

	m_isActivate = false;
}

void CEffect_Decal::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

}

void CEffect_Decal::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	EFFECT_INFO* pDesc = static_cast<EFFECT_INFO*>(pArg);

	m_isActivate = pDesc->IsActive;

	if (pDesc->pObjectMatrixPtr != nullptr && m_isActivate)
	{
		_fmatrix SpawnMatrix = XMLoadFloat4x4(pDesc->pObjectMatrixPtr);
		Default_Transform(SpawnMatrix, pDesc->OffsetMatrix);
	}
	else if (m_isActivate)
	{
		_fmatrix Spawnmatrix = XMMatrixIdentity();
		Default_Transform(Spawnmatrix, pDesc->OffsetMatrix);
	}
}

void CEffect_Decal::Default_Transform(_fmatrix SpawnMatrix, _fmatrix OffsetMatrix)
{
	//_vector vScale = {};
	//_vector vPos = {};
	//_vector vRot = {};
	//XMMatrixDecompose(&vScale, &vRot, &vPos, SpawnMatrix);

	//_matrix PositionMatrix = XMMatrixTranslationFromVector(vPos);


	m_ComBindMatrix = OffsetMatrix * SpawnMatrix;
}


CEffect_Decal* CEffect_Decal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEffect_Decal* pInstance = new CEffect_Decal(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEffect_Decal");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEffect_Decal::Clone(void* pArg)
{
    CEffect_Decal* pInstance = new CEffect_Decal(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CEffect_Decal");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEffect_Decal::Free()
{
    __super::Free();

}
