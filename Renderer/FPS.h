#pragma once 

#pragma comment(lib, "winmm.lib")
#include "typedef.h"
namespace wilson
{
	class FPS
	{
	public:
		inline int GetFps()
		{
			return m_fps;
		}
		void Frame();
		void Init();
		
		FPS() = default;
		FPS(const FPS&) = delete;
		~FPS() = default;

	private:
		UINT m_count;
		UINT m_fps;
		ULONG m_startTime;
	};
}

