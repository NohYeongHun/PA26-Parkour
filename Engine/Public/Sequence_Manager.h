#pragma once
#include "Base.h"

#include "Sequence.h"

NS_BEGIN(Engine)

class CSequence_Manager final : public CBase
{
private:
	explicit CSequence_Manager();
	virtual ~CSequence_Manager() = default;

public:
	void		Register_Sequence(const _wstring& strSequenceTag, const vector<SEQUENCE_ITEM_INFO>& Items, const vector<SEQUENCE_ITEM_DATA*>& ItemDatas, void* pDesc);
	void		Play_Sequence(const _wstring& strSequenceTag);

public:
	void		Update(_float fTimeDelta);

private:
	map<const _wstring, CSequence*>	m_Sequences;
	CSequence*									m_MainSequence = { nullptr };

private:
	CSequence*		Find_Sequence(const _wstring& strSequenceTag);

public:
	static CSequence_Manager* Create();
	virtual void Free() override;
};

NS_END