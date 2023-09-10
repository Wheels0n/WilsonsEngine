#pragma once 

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
		UINT m_fps;
		UINT m_count;
		ULONG m_startTime;
	};
}

