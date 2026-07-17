#pragma once
#include "Base.h"
#include "Client_Define.h"
#include "StateMachine.h"

NS_BEGIN(Client)

struct TRANSITION_RULE_DATA
{
	vector<_string>  WhenFlags;              // 모두 참이어야 매치 (AND)
	vector<_string>  WhenNotFlags;           // 모두 거짓이어야 매치
	_uint            iAnimGuard = UINT_MAX;  
	Engine::StateKey Next{ 0, 0 };           
	_uint            iNextAnim = UINT_MAX;   
	_float           fBlendOverride = -1.f;  
};

class CTransitionTable final : public CBase
{
private:
	explicit CTransitionTable() = default;
	virtual ~CTransitionTable() = default;

public:
	HRESULT Load(const _string& strFilePath);
	void    Reload();
	_uint   Get_Version() const { return m_iVersion; }
	const vector<TRANSITION_RULE_DATA>* Get_Rules(const Engine::StateKey& Key) const;
	const map<Engine::StateKey, vector<TRANSITION_RULE_DATA>>& Get_All() const { return m_Table; }
	const map<_uint, vector<TRANSITION_RULE_DATA>>& Get_AllCategories() const { return m_CategoryTable; }

private:
	HRESULT Parse(const _string& strFilePath,
	              map<Engine::StateKey, vector<TRANSITION_RULE_DATA>>& OutTable,
	              map<_uint, vector<TRANSITION_RULE_DATA>>& OutCategoryTable,
	              _string& strOutError);

private:
	_string m_strFilePath;
	map<Engine::StateKey, vector<TRANSITION_RULE_DATA>> m_Table;
	map<_uint, vector<TRANSITION_RULE_DATA>> m_CategoryTable;
	_uint m_iVersion = 1;

public:
	static CTransitionTable* Create(const _string& strFilePath);
	virtual void Free() override;
};

NS_END
