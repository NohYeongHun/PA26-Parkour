#pragma once

#include "Timer.h"

NS_BEGIN(Engine)

class CTimer_Manager final : public CBase
{
private:
	explicit CTimer_Manager();
	virtual ~CTimer_Manager() = default;

public:
	_float			Get_TimeDelta(const _wstring& strTimerTag);
	void			Change_TimeRate(const _wstring& strTimerTag, _float fTimeRate);
	void			Change_TimeRate(const _wstring& strTimerTag, _float fTimeRate, _float fDuration);
	_double		Get_PlayTime() { return m_fPlayTime; }

public:
	void			Update(_float fTimeDelta);

	HRESULT		Add_Timer(const _wstring& strTimerTag);

private:		
	map<const _wstring, CTimer*>	m_Timers;
	_double									m_fPlayTime = {};

private:
	CTimer*		Find_Timer(const _wstring& strTimerTag);

public:
	static		CTimer_Manager* Create();
	virtual		void					Free();
};

NS_END