#ifndef FPS_H
#define FPS_H

#pragma comment(lib, "winmm.lib")

#include<windows.h>
#include<mmsyscom.h>

class CFps
{
public:
	CFps() = default;
	CFps(const CFps&) = delete;
	~CFps() = default;

	void Init();
	void Frame();
	int GetFps();
private:
	int m_fps;
	int m_count;
	unsigned long m_startTime;
};

#endif // !FPS_H
