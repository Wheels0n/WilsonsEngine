#ifndef TIMER_H
#define TIMER_H
#include<Windows.h>
class Timer
{
public:
	void Reset();
	void Start();
	void Stop();
	void Tick();

	Timer();
	Timer(const Timer&) = delete;
	~Timer() = default;
private:
	float m_secPerTick;
	float m_dt;

	__int64 m_baseTime;
	__int64 m_pausedTime;
	__int64 m_stopTime;
	__int64 m_prevTime;
	__int64 m_curTime;

	bool m_bStopped;
};
#endif // !TIMER_H
