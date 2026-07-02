#include"ClientPch.h"
#include "MapObject_FireFly.h"

CMapObject_FireFly::CMapObject_FireFly(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice,pContext)
{
}

CMapObject_FireFly::CMapObject_FireFly(const CMapObject_FireFly& Prototype)
	:CGameObject(Prototype)
{
}

HRESULT CMapObject_FireFly::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapObject_FireFly::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	Ready_Component(pArg);
    return S_OK;
}

void CMapObject_FireFly::Priority_Update(_float fTimeDelta)
{
}

void CMapObject_FireFly::Update(_float fTimeDelta)
{
	m_fTotalTime += fTimeDelta;
	if (m_fTotalTime >= 3600.f)
		m_fTotalTime -= 3600.f;
}

void CMapObject_FireFly::Late_Update(_float fTimeDelta)
{
	if (XMVectorGetX(XMVector3Length(XMVectorSetY(m_pTransformCom->Get_State(STATE::POSITION) - XMLoadFloat4(m_pGameInstance->Get_CamPos()), 0.f))) <= 100.f)
		m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CMapObject_FireFly::Render()
{
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", 0, TEXTURETYPE::DIFFUSE, 0);
	m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", 0, TEXTURETYPE::NORMAL, 0);

	m_pShaderCom->Bind_Value("g_fRaidan", &m_fTotalTime, sizeof(_float));

	m_pShaderCom->Begin(m_iShaderPassIndex);
	m_pModelCom->Render(0);

}

void CMapObject_FireFly::Render_Shadow()
{
}

void CMapObject_FireFly::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	if (FAILED(__super::Add_Component(pDesc->iLevel, StringToWString(pDesc->ModelName),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Failed");

	if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Shader_FireFly"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Failed");

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->WorldMatrix));
}

CMapObject_FireFly* CMapObject_FireFly::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_FireFly* pInstance = new CMapObject_FireFly(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype())) {
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_FireFly::Clone(void* pArg)
{
	CMapObject_FireFly* pInstance = new CMapObject_FireFly(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg))) {
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMapObject_FireFly::Free()
{
	__super::Free();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
