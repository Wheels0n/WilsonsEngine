#include "FPS.h"

namespace wilson
{
	void FPS::Init()
	{
		m_fps = 0;
		m_count = 0;
		m_startTime = timeGetTime();
		return;
	}

	void FPS::Frame()
	{
		++m_count;

		if (timeGetTime() >= (m_startTime + 1000))
		{
			m_fps = m_count;
			m_count = 0;
			m_startTime = timeGetTime();
		}

		return;

	}

}