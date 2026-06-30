#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CTimer final : public CBase
{
private:
	explicit CTimer();
	virtual ~CTimer() = default;

public:
	_float	Get_TimeDelta() const { return m_fTimeDelta * m_fTimeRate; }
	void	Change_TimeRate(_float fTimeRate) { m_fTimeRate = fTimeRate; }
	void	Change_TimeRate(_float fTimeRate, _float fDuration) {
		m_fTimeRate = fTimeRate;
		m_fDuration = fDuration;
	}

public:
	HRESULT		Initialize();
	void			Update(_float fTimeDelta);
	void			Update_Timer();

private:
	LARGE_INTEGER		m_FrameTime = {};
	LARGE_INTEGER		m_FixTime = {};
	LARGE_INTEGER		m_LastTime = {};
	LARGE_INTEGER		m_CpuTick = {};

	_float						m_fTimeDelta = {};
	_float						m_fTimeRate = { 1.f };
	_float						m_fDuration = {};

public:
	static CTimer* Create();
	virtual void Free() override;
};

NS_END

