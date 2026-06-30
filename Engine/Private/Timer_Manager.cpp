#include "EnginePch.h"
#include "Timer_Manager.h"

CTimer_Manager::CTimer_Manager()
{

}

_float CTimer_Manager::Get_TimeDelta(const _wstring& strTimerTag)
{
	CTimer* pTimer = Find_Timer(strTimerTag);
	if (nullptr == pTimer)
		return 0.f;

	pTimer->Update_Timer();
	return pTimer->Get_TimeDelta();
}

void CTimer_Manager::Change_TimeRate(const _wstring& strTimerTag, _float fTimeRate)
{
	CTimer* pTimer = Find_Timer(strTimerTag);
	if (nullptr == pTimer)
		return;

	pTimer->Change_TimeRate(fTimeRate);
}

void CTimer_Manager::Change_TimeRate(const _wstring& strTimerTag, _float fTimeRate, _float fDuration)
{
	CTimer* pTimer = Find_Timer(strTimerTag);
	if (nullptr == pTimer)
		return;

	pTimer->Change_TimeRate(fTimeRate, fDuration);
}

void CTimer_Manager::Update(_float fTimeDelta)
{
	m_fPlayTime += fTimeDelta;

	for (auto& Pair : m_Timers)
		Pair.second->Update(fTimeDelta);
}

HRESULT CTimer_Manager::Add_Timer(const _wstring& strTimerTag)
{
	CTimer* pTimer = Find_Timer(strTimerTag);

	if (nullptr != pTimer)
		return E_FAIL;

	pTimer = CTimer::Create();
	if (nullptr == pTimer)
		return E_FAIL;

	m_Timers.emplace(strTimerTag, pTimer);

	return S_OK;
}

CTimer* CTimer_Manager::Find_Timer(const _wstring& strTimerTag)
{
	auto iter = m_Timers.find(strTimerTag);

	if (iter == m_Timers.end())
		return nullptr;

	return iter->second;
}

CTimer_Manager* CTimer_Manager::Create()
{
	return new CTimer_Manager();
}

void CTimer_Manager::Free()
{
	__super::Free();

	for (auto& Pair : m_Timers)	
		Safe_Release(Pair.second);
	m_Timers.clear();
}
