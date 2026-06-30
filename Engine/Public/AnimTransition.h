#pragma once

#include "Base.h"

NS_BEGIN(Engine)
class CAnimTransition final : public CBase
{
public:
	//enum CONDITOIN_TYPE {SINGLE, MULTI};
	typedef struct tagTransitionData
	{
		_string strFrom;
		_string strTo;
		_int iPriority;
		//조건에 사용할 const flag변수
		_uint iTargetState;
		//다음 애니메이션의 특정 트랙위치로
		_float fTargetTrackPos;
		// 현재 애니메이션에서 변환할 수 있는 트랙위치
		_float fTransitEnablePos;

		_uint eType;
	}TRANSITION_DESC;
	typedef function<_bool(const _uint*, const _uint)> CONDITION;

private:
	explicit CAnimTransition();
	virtual ~CAnimTransition() = default;

public:
	const _float Get_TransitEnablePos() { return m_fTransitEnablePos; }
	const _int Get_Priority() { return m_iPriority; }
	void Get_ToStateData(_string& strToState, _float& fTargetTrackPos) {
		strToState = m_strNextState;
		fTargetTrackPos = m_fTargetTrackPos;
	}
public:
	HRESULT Initialize_Prototype(json& jsonParser, CONDITION Func);

	//_bool Is_Transit(const _uint* pOwnerState, _int& iAnimStateIndex);
	_bool Is_Transit(const _uint* pOwnerState, _string& strNextState, _float& fTargetTrackPos);

#ifdef _DEBUG
	void Get_TransitData(_string& strNextState, _uint& iTargetState, _float& fTargetTrackPos)
	{
		strNextState = m_strNextState;
		iTargetState = m_iTargetState;
		fTargetTrackPos = m_fTargetTrackPos;
	}
#endif // DEBUG

private:
	_string m_strNextState;
	//_int m_iNextStateIndex{ -1 };

	_int m_iPriority{};
	//조건에 사용할 const flag변수
	_uint m_iTargetState{};
	_float m_fTargetTrackPos{};
	_float m_fTransitEnablePos{};
	//typedef vector<function<_bool(const _uint*)>> CONDITION;
	//CONDITION m_Conditions;
	CONDITION m_Condition;

public:
	static CAnimTransition* Create(json& jsonParser, CONDITION Func);
#ifdef _DEBUG
	static CAnimTransition* Create();
#endif // _DEBUG
	virtual void Free() override;
};
NS_END
