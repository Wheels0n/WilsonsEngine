#include "Fps.h"
void CFps::Init()
{
	m_fps = 0;
	m_count = 0;
	m_startTime = timeGetTime();
	return;
}

void CFps::Frame()
{
	++m_count;

	if (timeGetTime() >= (m_startTime + 1000))
	{
		m_fps = m_count;
		m_count = 0;
		m_startTime = timeGetTime();
	}

	ImGui::Begin("Utilization", nullptr, ImGuiWindowFlags_MenuBar);
	ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS:%d", m_fps);
	ImGui::End();

	return;

}

int CFps::GetFps()
{
	return m_fps;
}
