#include "EnginePch.h"
#include "Timer.h"

CTimer::CTimer() 
{
	ZeroMemory(&m_FixTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&m_LastTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&m_FrameTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&m_CpuTick, sizeof(LARGE_INTEGER));
}


HRESULT CTimer::Initialize()
{
	// ?꾩옱 CPU 移댁슫???レ옄
	QueryPerformanceCounter(&m_FrameTime);			// 1077
	QueryPerformanceCounter(&m_LastTime);			// 1085
	QueryPerformanceCounter(&m_FixTime);			// 1090

	// 珥덈떦 CPU媛 移댁슫???????덈뒗 理쒕? ?レ옄
	QueryPerformanceFrequency(&m_CpuTick);

	return S_OK;
}

void CTimer::Update(_float fTimeDelta)
{
	if (0.f < m_fDuration)
		m_fDuration -= fTimeDelta;
	else
		m_fTimeRate = 1.f;
}

void CTimer::Update_Timer()
{
	QueryPerformanceCounter(&m_FrameTime);			// 1500

	if (m_FrameTime.QuadPart - m_FixTime.QuadPart >= m_CpuTick.QuadPart)
	{
		QueryPerformanceFrequency(&m_CpuTick);
		m_FixTime = m_FrameTime;
	}

	m_fTimeDelta = (m_FrameTime.QuadPart - m_LastTime.QuadPart) / static_cast<_float>(m_CpuTick.QuadPart);

	m_LastTime = m_FrameTime;
}

CTimer* CTimer::Create()
{
	CTimer* pInstance = new CTimer();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : Timer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTimer::Free()
{
	__super::Free();
}
