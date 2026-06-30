#include "EnginePch.h"
#include "Sequence.h"

#include "GameInstance.h"

CSequence::CSequence()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CSequence::Initialize(const vector<SEQUENCE_ITEM_INFO>& Items, const vector<SEQUENCE_ITEM_DATA*>& ItemDatas, void* pArg)
{
	ASSERT_CRASH(Items.size() == ItemDatas.size());

	for (auto& Item : Items)
		m_Items.push_back(Item);
	//memcpy(m_Items.data(), Items.data(), sizeof(SEQUENCE_ITEM_INFO) * Items.size());

	for (auto& Data : ItemDatas)
		m_ItemDatas.push_back(Data);

	//memcpy(m_ItemDatas.data(), ItemDatas.data(), sizeof(ItemDatas));

	SEQUENCE_DESC* pDesc = static_cast<SEQUENCE_DESC*>(pArg);
	m_fEndFrame = pDesc->fDuration;
	m_fTrackPerSec = pDesc->fTrackPerSec;

	return S_OK;
}

_bool CSequence::Update(_float fTimeDelta)
{
	// Sequence End
	if (m_fTrackPosition > m_fEndFrame)
	{
		m_fTrackPosition = 0.f;
		m_iItemIndex = 0;
		return true;
	}

	m_fTrackPosition += m_fTrackPerSec * fTimeDelta;

	// All Item Spawn
	if (m_iItemIndex >= m_Items.size())
		return false;

	if (m_fTrackPosition > m_Items[m_iItemIndex].fStartFrame)
	{
		if (ITEM_TYPE::SCENE == m_Items[m_iItemIndex].eType)
			m_pGameInstance->Change_MainCamera(ENUM_CLASS(m_pGameInstance->Get_CurrentLevel()), m_Items[m_iItemIndex].strItemTag, m_ItemDatas[m_iItemIndex]);
		else
			m_pGameInstance->Spawn_PoolingObject(m_Items[m_iItemIndex].strItemTag, XMMatrixIdentity(), &m_ItemDatas[m_iItemIndex]);

		++m_iItemIndex;
	}

	return false;
}

CSequence* CSequence::Create(const vector<SEQUENCE_ITEM_INFO>& Items, const vector<SEQUENCE_ITEM_DATA*>& ItemDatas, void* pDesc)
{
	CSequence* pInstance = new CSequence();

	if (FAILED(pInstance->Initialize(Items, ItemDatas, pDesc)))
		CRASH("SQ");

	return pInstance;
}

void CSequence::Free()
{
	__super::Free();

	m_Items.clear();

	for (auto& pData : m_ItemDatas)
		Safe_Delete(pData);
	m_ItemDatas.clear();

	Safe_Release(m_pGameInstance);
}
