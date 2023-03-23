#ifndef FPS_H
#define FPS_H

#pragma comment(lib, "winmm.lib")

#include"../ImGui/imgui.h"
#include<windows.h>
#include<mmsyscom.h>

namespace wilson
{
	class FPS
	{
	public:
		void Init();
		void Frame();
		
		inline int GetFps()
		{
			return m_fps;
		}

		FPS() = default;
		FPS(const FPS&) = delete;
		~FPS() = default;

	private:
		int m_fps;
		int m_count;
		unsigned long m_startTime;
	};
}
#endif // !FPS_H
