#include "Timer.h"

Timer::Timer()
{
	m_secPerTick = 0.0f;
	m_dt = -1.0f;
	m_baseTime = 0.0f;
	m_pausedTime = 0.0f;
	m_stopTime = 0.0f;
	m_prevTime = 0.0f;
	m_curTime = 0.0f;
	m_bStopped = false;

	__int64 ticksPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&ticksPerSec);
	m_secPerTick = 1.0f / (float)ticksPerSec;
}

void Timer::Reset()
{
	__int64 curTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTime);

	m_baseTime = curTime;
	m_prevTime = curTime;
	m_stopTime = 0;
	m_bStopped = false;
}

void Timer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*) & startTime);

	if (m_bStopped)
	{
		m_pausedTime += (startTime - m_stopTime);
		m_prevTime = startTime;
		m_stopTime = 0;
		m_bStopped = false;
	}
}

void Timer::Stop()
{
	if (!m_bStopped)
	{
		__int64 curTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&curTime);

		m_stopTime = curTime;
		m_bStopped = true;
	}
}

void Timer::Tick()
{
	if (m_bStopped)
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
