#include"ClientPch.h"
#include "MapObject_Meteo.h"
#include"GameSystem.h"

vector<_wstring> CMapObject_Meteo::m_SoundTags;

CMapObject_Meteo::CMapObject_Meteo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice,pContext)
{
}

CMapObject_Meteo::CMapObject_Meteo(const CMapObject_Meteo& Prototype)
	:CGameObject(Prototype), m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMapObject_Meteo::Initialize_Prototype()
{
	m_SoundTags.push_back(TEXT("Explosion0"));
	m_SoundTags.push_back(TEXT("Explosion1"));
	m_SoundTags.push_back(TEXT("Explosion2"));

    return S_OK;
}

HRESULT CMapObject_Meteo::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	m_iSoundChannel = m_pGameInstance->Register_Channel();
	Ready_Components(pArg);
	
	_float2 vRand = _float2(130.f, 180.f);
	m_vRadians = _float3(XMConvertToRadians(m_pGameInstance->Rand(vRand.x, vRand.y)), XMConvertToRadians(m_pGameInstance->Rand(vRand.x, vRand.y)), XMConvertToRadians(m_pGameInstance->Rand(vRand.x, vRand.y)));
    return S_OK;
}

void CMapObject_Meteo::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Turn_Quaternion(m_vRadians, fTimeDelta);
}

void CMapObject_Meteo::Update(_float fTimeDelta)
{
	if (m_IsTriggerd)
		LerpPos(fTimeDelta);
}

void CMapObject_Meteo::Late_Update(_float fTimeDelta)
{
	if (m_IsTriggerd)
		m_pGameInstance->Add_Render_Object(RENDERGROUP::NONSTATIC, this);
}

void CMapObject_Meteo::Render()
{
	_uint m_iLODIndex = 0;
	if (m_iLODIndex > m_pModelCom->Get_LastLODIndex())
		return;

	_bool HasNormal = { true };
	_bool HasMask = { true };
	_uint iNumMesh = m_pModelCom->Get_NumMesh(m_iLODIndex);

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	m_pModelCom->Bind_Buffer(m_pContext, m_iLODIndex);
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (m_pModelCom->Is_Overed(m_iLODIndex, i))
			return;
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", m_iLODIndex, i, TEXTURETYPE::MASK)))
		{
			m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
			HasMask = false;
		}

		if (HasMask)
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL)))
				HasNormal = false;
		}
		else
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE, 0);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL, 0)))
				HasNormal = false;
		}
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));

		m_pShaderCom->Begin(28);
		m_pModelCom->Render(m_iLODIndex, i);
	}
}

void CMapObject_Meteo::LerpPos(_float fTimeDelta)
{
	if (!m_IsSound)
	{
		m_pGameInstance->Play_Sound_Dynamic(TEXT("Fire_Long"), m_iSoundChannel, 0.5f);
		m_IsSound = !m_IsSound;
	}
	m_fFall += fTimeDelta;
	_float Time = m_fFall / m_fDuration;
	_vector current_xz = XMVectorLerp(XMLoadFloat4(&m_vSourPos), XMLoadFloat4(&m_vDestPos), Time);

	_float y_arc = sin(Time * XM_PI) * m_farchY + current_xz.m128_f32[1];
	_vector CurrentPos = XMVectorSetY(current_xz, y_arc);

	m_pTransformCom->Set_State(STATE::POSITION, CurrentPos);
	if (Time >= 1.f)
	{
		m_ISTrailEffect = false;
		m_IsTriggerd = false;
		m_isActivate = false;

		PREFAB_INFO Info{};
		m_pGameInstance->Spawn_PoolingObject(TEXT("Explosion"), m_pTransformCom->Get_WorldMatrix(), &Info);
		m_pGameInstance->Spawn_PoolingObject(TEXT("Big_Smoke"), m_pTransformCom->Get_WorldMatrix(), &Info);
		//이펙트들 터트리기.
		_uint SoundChannel = m_pGameInstance->Register_Channel();
		switch (static_cast<_uint>(m_pGameInstance->Rand(0.f, 3.f)))
		{
		case 0:
			m_pGameInstance->Play_Sound_Dynamic(m_SoundTags[0], SoundChannel, 0.2f);
			break;

		case 1:
			m_pGameInstance->Play_Sound_Dynamic(m_SoundTags[1], SoundChannel, 0.2f);
			break;

		case 2:
			m_pGameInstance->Play_Sound_Dynamic(m_SoundTags[2], SoundChannel, 0.2f);
			break;
		}

		m_pGameInstance->Return_Channel(SoundChannel);
		m_pGameInstance->Stop_Sound_Dynamic(m_iSoundChannel);
		m_pGameInstance->Return_Channel(m_iSoundChannel);

		//m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&m_vSourPos));
		m_fFall = 0.f;
	}
}

void CMapObject_Meteo::Ready_Components(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	m_pTransformCom->Set_WorldMatrix(XMMatrixScaling(0.4f, 0.4f, 0.4f) * XMLoadFloat4x4(&pDesc->WorldMatrix));

	m_farchY = pDesc->fArchY;
	m_fDuration = pDesc->fDuration;
	m_vDestPos = pDesc->vDestPos;
	m_vSourPos = pDesc->vSourPos;
	m_iTriggerIndex = pDesc->TriggerIndex;
	m_iTriggerActiveIndex = pDesc->TriggerActiveIndex;

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	lstrcat(Model, StringToWString(pDesc->ModelName).c_str());

	m_iShaderPassIndex = pDesc->iShaderPassIndex;

	_wstring WModelName = Model;
	WModelName.pop_back();
	WModelName.pop_back();
	WModelName.pop_back();
	WModelName.pop_back();
	WModelName.pop_back();
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->iLevel), WModelName,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return;

}

CMapObject_Meteo* CMapObject_Meteo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Meteo* pInstance = new CMapObject_Meteo(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject_Meteo");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Meteo::Clone(void* pArg)
{
	CMapObject_Meteo* pInstance = new CMapObject_Meteo(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject_Meteo (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMapObject_Meteo::Free()
{
	__super::Free();
	m_pTempPtr = nullptr;
	m_pSecondTempPtr = nullptr;
	m_pThirdTempPtr = nullptr;
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pGameSystem);
	Safe_Release(m_pModelCom);

}
