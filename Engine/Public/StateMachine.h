#pragma once
#include "Component.h"


NS_BEGIN(Engine)

// 계층적 State Key 관리
struct StateKey
{
	_uint iCategory;	// 상위 카테고리 (Ground, Air, Climb, Hit)
	_uint iSubState;	// 하위 상태 (Idle, Run, Walk, etc.)

	StateKey(_uint category, _uint subState)
		: iCategory(category), iSubState(subState) {}

	// map에서 사용하기 위한 비교 연산자
	bool operator<(const StateKey& other) const
	{
		if (iCategory != other.iCategory)
			return iCategory < other.iCategory;
		return iSubState < other.iSubState;
	}

	// 단일 _uint로 변환 (상위 8bit: Category, 하위 24bit: SubState)
	_uint ToUInt() const
	{
		return (iCategory << 8) | (iSubState & 0x00FFFFFF);
	}

	static StateKey FromUInt(_uint key)
	{
		return StateKey(key >> 24, key & 0x00FFFFFF);
	}
};

class ENGINE_DLL CStateMachine : public CComponent
{
#pragma region 기본 함수
public:
	explicit CStateMachine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CStateMachine(const CStateMachine& Prototype);
	virtual ~CStateMachine() = default;

public:
	virtual HRESULT		Initialize_Prototype();
	virtual HRESULT		Initialize_Clone(void* pArg);
#pragma endregion


public:
	virtual void Update(_float fTimeDelta);

	// HSM: 계층적 State 전환 (Category + SubState)
	/*void Change_State(_uint iCategory, _uint iSubState, void* pArg = nullptr);
	void Change_State(const StateKey& key, void* pArg = nullptr);*/
	void Change_State(_uint iCategory, _uint iSubState, void* pArg = nullptr);
	void Change_State(const StateKey& key, void* pArg = nullptr);

	// State 추가
	void Add_State(_uint iCategory, _uint iSubState, class CState* pState);
	void Add_State(const StateKey& key, class CState* pState);

	// 현재 State 탈출
	void Exit_State();

	// 현재 State 정보 반환
	StateKey Get_CurrentStateKey() const { return m_CurrentStateKey; }
	_uint Get_CurrentCategory() const { return m_CurrentStateKey.iCategory; }
	_uint Get_CurrentSubState() const { return m_CurrentStateKey.iSubState; }

private:
	map<StateKey, class CState*> m_States;	// HSM: Category별로 구분된 State들
	StateKey m_CurrentStateKey = { 0, 0 };	// 현재 활성화된 State (Category + SubState)
	class CState* m_pCurrentState = { nullptr };

public:
	static CStateMachine* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};
NS_END
