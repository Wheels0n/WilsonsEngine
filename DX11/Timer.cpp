#include "Timer.h"

CTimer::CTimer()
{
	m_secPerTick = 0.0f;
	m_dt = -1.0f;
	m_baseTime = 0.0f;
	m_pausedTime = 0.0f;
	m_stopTime = 0.0f;
	m_prevTime = 0.0f;
	m_curTime = 0.0f;
	m_isStopped = false;

	__int64 ticksPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&ticksPerSec);
	m_secPerTick = 1.0f / (float)ticksPerSec;
}

void CTimer::Reset()
{
	__int64 curTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTime);

	m_baseTime = curTime;
	m_prevTime = curTime;
	m_stopTime = 0;
	m_isStopped = false;
}

void CTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*) & startTime);

	if (m_isStopped)
	{
		m_pausedTime += (startTime - m_stopTime);
		m_prevTime = startTime;
		m_stopTime = 0;
		m_isStopped = false;
	}
}

void CTimer::Stop()
{
	if (!m_isStopped)
	{
		__int64 curTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&curTime);

		m_stopTime = curTime;
		m_isStopped = true;
	}
}

void CTimer::Tick()
{
	if (m_isStopped)
	{
		m_dt = 0.0f;
		return;
	}

	__int64 curTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTime);
	m_curTime = curTime;

	m_dt = (m_curTime - m_prevTime) * m_secPerTick;
	m_prevTime = m_curTime;

	if (m_dt < 0.0f)
	{
		m_dt = 0.0f;
	}
}
