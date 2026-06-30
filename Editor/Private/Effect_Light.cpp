#include "Editorpch.h"
#include "Effect_Light.h"

CEffect_Light::CEffect_Light(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CEffect_Light::CEffect_Light(const CEffect_Light& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CEffect_Light::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Light::Initialize_Clone(void* pArg)
{
	LIGHT_DESC* pDesc = static_cast<LIGHT_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

	m_tDesc = *pDesc;

    m_vColor = pDesc->vColor;
    m_vLifeTime = pDesc->vLifeTime;
	m_wstrLightTag = pDesc->wstrLightTag;
	m_fSpeed = pDesc->fSpeed;
	m_vRange = pDesc->vRange;
	m_fAmbient = pDesc->fAmbient;

    m_isActivate = false;

    return S_OK;
}

void CEffect_Light::Priority_Update(_float fTimeDelta)
{
}

void CEffect_Light::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	if (m_vLifeTime.x >= m_vLifeTime.y)
	{
		m_isActivate = false;
		m_pGameInstance->Set_LightActive(m_wstrLightTag, false);

		//초기화
		m_vLifeTime.x = 0.f;
		

		return;
	}

	m_vLifeTime.x += fTimeDelta;

	Update_LightDesc(fTimeDelta);

	m_pGameInstance->Update_LightDesc(m_wstrLightTag, m_tLightDesc);
}

void CEffect_Light::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

}

void CEffect_Light::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	EFFECT_INFO* pDesc = static_cast<EFFECT_INFO*>(pArg);

	//초기화
	m_vLifeTime.x = 0.f;

	if (m_isActivate = pDesc->IsActive)
	{
		_vector vScale = {};
		_vector vTrans = {};
		_vector vRot = {};
		XMMatrixDecompose(&vScale, &vRot, &vTrans, WorldMatrix);

		_float4 vPos = {};
		XMStoreFloat4(&vPos, vTrans);

		m_tLightDesc.eType = Engine::LIGHT_DESC::POINT;
		m_tLightDesc.vDiffuse = m_tDesc.vColor;
		m_tLightDesc.vPosition = vPos;
		m_tLightDesc.fRange = m_vRange.x;
		m_tLightDesc.vAmbient = _float4(m_fAmbient, m_fAmbient, m_fAmbient, 1.f);

		m_pGameInstance->Update_LightDesc(m_wstrLightTag, m_tLightDesc);
		m_pGameInstance->Set_LightActive(m_wstrLightTag, true);
	}
}

void CEffect_Light::Update_LightDesc(_float fTimeDelta)
{
	if (m_tLightDesc.fRange < m_vRange.y)
	{
		m_tLightDesc.fRange += fTimeDelta * m_fSpeed;
	}
}

CEffect_Light* CEffect_Light::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEffect_Light* pInstance = new CEffect_Light(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEffect_Light");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEffect_Light::Clone(void* pArg)
{
    CEffect_Light* pInstance = new CEffect_Light(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CEffect_Light");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEffect_Light::Free()
{
    __super::Free();

}
