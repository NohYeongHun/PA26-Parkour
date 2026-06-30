#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CSequence final : public CBase
{
public:
	typedef struct tagSequenceDesc
	{
		_float fDuration = { };
		_float fTrackPerSec = {};
	}SEQUENCE_DESC;
private:
	explicit CSequence();
	virtual ~CSequence() = default;

public:
	HRESULT						Initialize(const vector<SEQUENCE_ITEM_INFO>& Items, const vector<SEQUENCE_ITEM_DATA*>& ItemDatas, void* pArg);
	_bool							Update(_float fTimeDelta);

private:
	class CGameInstance*				m_pGameInstance = { nullptr };

	_float										m_fStartFrame{}, m_fEndFrame{};
	_int										m_iItemIndex = {};
	vector<SEQUENCE_ITEM_INFO>	m_Items;
	vector<SEQUENCE_ITEM_DATA*>	m_ItemDatas;

	_float										m_fTrackPerSec = {};
	_float										m_fTrackPosition = {};

public:
	static		CSequence*		Create(const vector<SEQUENCE_ITEM_INFO>& Items, const vector<SEQUENCE_ITEM_DATA*>& ItemDatas, void* pDesc);
	virtual		void				Free() override;
};

NS_END