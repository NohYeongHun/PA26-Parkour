#include"ClientPch.h"
#include "MapObject_DynamicSound.h"

vector<_wstring> CMapObject_DynamicSound::m_SoundTags;

CMapObject_DynamicSound::CMapObject_DynamicSound(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice,pContext)
{
}

CMapObject_DynamicSound::CMapObject_DynamicSound(const CMapObject_DynamicSound& Prototype)
	:CGameObject(Prototype)
{
}

HRESULT CMapObject_DynamicSound::Initialize_Prototype()
{
	m_SoundTags.push_back(TEXT("Fire_Long0"));
	m_SoundTags.push_back(TEXT("Fire_Long0"));
	m_SoundTags.push_back(TEXT("Fire_Long01"));
	return S_OK;
}

HRESULT CMapObject_DynamicSound::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	SOUND_DESC* pDesc= static_cast<SOUND_DESC*>(pArg);

	m_iSoundChannel = m_pGameInstance->Register_Channel();
	m_iSoundIndex = pDesc->iSoundIndex;
	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->SoundMat));
	
	switch (m_iSoundIndex)
	{
	case 0:
		//불
		m_fSoundCoolDown = 8.f;
		m_fSoundVolume = 0.15f;
		break;
	case 1:
		//불
		m_fSoundCoolDown = 8.f;
		m_fSoundVolume = 0.15f;
		break;
	case 2:
		//모닥불
		m_fSoundCoolDown = 7.f;
		m_fSoundVolume = 0.25f;
		break;
	}
	
	return S_OK;
}

void CMapObject_DynamicSound::Update(_float fTimeDelta)
{
		m_fTotalTime += fTimeDelta;
		if (m_fTotalTime >= m_fSoundCoolDown)
		{
			m_fTotalTime = 0.f;
			//if (XMVectorGetX(XMVector3Length(XMVectorSetY(m_pTransformCom->Get_State(STATE::POSITION) - XMLoadFloat4(m_pGameInstance->Get_CamPos()), 0.f))) < 500.f)
				m_pGameInstance->Play_Sound_Dynamic(m_SoundTags[m_iSoundIndex], m_iSoundChannel, m_fSoundVolume, m_pTransformCom, 0.1f, 20.f);
		}
}

CMapObject_DynamicSound* CMapObject_DynamicSound::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_DynamicSound* pInstance = new CMapObject_DynamicSound(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CMapObject_DynamicSound");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_DynamicSound::Clone(void* pArg)
{
	CMapObject_DynamicSound* pClone = new CMapObject_DynamicSound(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CMapObject_DynamicSound (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMapObject_DynamicSound::Free()
{
	__super::Free();
	if (m_iSoundChannel != -1)
		m_pGameInstance->Return_Channel(m_iSoundChannel);
}