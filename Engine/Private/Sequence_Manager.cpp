#include "EnginePch.h"
#include "Sequence_Manager.h"

CSequence_Manager::CSequence_Manager()
{
}

void CSequence_Manager::Register_Sequence(const _wstring& strSequenceTag, const vector<SEQUENCE_ITEM_INFO>& Items, const vector<SEQUENCE_ITEM_DATA*>& ItemDatas, void* pDesc)
{
	CSequence* pSequence = Find_Sequence(strSequenceTag);
	if (nullptr != pSequence)
	{
		MSG_BOX("Sequence 이미 있음");
		return;
	}

	pSequence = CSequence::Create(Items, ItemDatas, pDesc);
	ASSERT_CRASH(pSequence);

	m_Sequences.emplace(strSequenceTag, pSequence);
}

void CSequence_Manager::Play_Sequence(const _wstring& strSequenceTag)
{
	// Playing Other Sequence
	if (nullptr != m_MainSequence)
		return;

	CSequence* pSequence = Find_Sequence(strSequenceTag);
	if (nullptr == pSequence)
		return;

	m_MainSequence = pSequence;
	Safe_AddRef(m_MainSequence);
}

void CSequence_Manager::Update(_float fTimeDelta)
{
	if (nullptr == m_MainSequence)
		return;

	_bool isEnd = m_MainSequence->Update(fTimeDelta);
	if (true == isEnd)
	{
		Safe_Release(m_MainSequence);
		m_MainSequence = nullptr;
	}
}

CSequence* CSequence_Manager::Find_Sequence(const _wstring& strSequenceTag)
{
	auto iter = m_Sequences.find(strSequenceTag);

	if (iter == m_Sequences.end())
		return nullptr;

	return iter->second;
}

CSequence_Manager* CSequence_Manager::Create()
{
    return new CSequence_Manager();
}

void CSequence_Manager::Free()
{
	__super::Free();

	for (auto& Pair : m_Sequences)
		Safe_Release(Pair.second);
	m_Sequences.clear();

	Safe_Release(m_MainSequence);
}
