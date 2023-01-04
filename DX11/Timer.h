#ifndef TIMER_H
#define TIMER_H
#include<Windows.h>
class CTimer
{
public:
	CTimer();
	CTimer(const CTimer&) = delete;
	~CTimer() = default;

	void Reset();
	void Start();
	void Stop();
	void Tick();

private:
	float m_secPerTick;
	float m_dt;

	__int64 m_baseTime;
	__int64 m_pausedTime;
	__int64 m_stopTime;
	__int64 m_prevTime;
	__int64 m_curTime;

	bool m_isStopped;
};
#endif // !TIMER_H
