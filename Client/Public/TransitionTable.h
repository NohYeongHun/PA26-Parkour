#pragma once
#include "Base.h"
#include "Client_Define.h"
#include "StateMachine.h"

NS_BEGIN(Client)

// JSON에서 읽은 전환 규칙 1개 (상태/애님 이름은 로드 시 인덱스로 해석 완료,
// 플래그 이름은 상태 인스턴스가 바인딩 시 슬롯으로 해석)
struct TRANSITION_RULE_DATA
{
	vector<_string>  WhenFlags;              // 모두 참이어야 매치 (AND)
	vector<_string>  WhenNotFlags;           // 모두 거짓이어야 매치
	_uint            iAnimGuard = UINT_MAX;  // 현재 재생 애님 가드 (UINT_MAX = 없음)
	Engine::StateKey Next{ 0, 0 };           // 다음 상태
	_uint            iNextAnim = UINT_MAX;   // 진입 애님 (UINT_MAX = 대상 상태 기본값)
	_float           fBlendOverride = -1.f;  // 크로스페이드 시간 오버라이드 (< 0 = 없음, 0 = 스냅)
};

class CTransitionTable final : public CBase
{
private:
	explicit CTransitionTable() = default;
	virtual ~CTransitionTable() = default;

public:
	HRESULT Load(const _string& strFilePath);
	void    Reload(); // 실패 시 이전 테이블 유지 (게임 계속 진행)
	_uint   Get_Version() const { return m_iVersion; }
	// 반환 포인터는 다음 Reload() 성공 시까지만 유효 - Get_Version() 변경 감지 시 재취득할 것
	const vector<TRANSITION_RULE_DATA>* Get_Rules(const Engine::StateKey& Key) const;

private:
	HRESULT Parse(const _string& strFilePath,
	              map<Engine::StateKey, vector<TRANSITION_RULE_DATA>>& OutTable,
	              _string& strOutError);

private:
	_string m_strFilePath;
	map<Engine::StateKey, vector<TRANSITION_RULE_DATA>> m_Table;
	_uint m_iVersion = 1;

public:
	static CTransitionTable* Create(const _string& strFilePath);
	virtual void Free() override;
};

NS_END
