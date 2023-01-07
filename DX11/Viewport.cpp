#include "Viewport.h"

void CViewport::Init(ID3D11ShaderResourceView* pSRV)
{
	m_pSRV = pSRV;
}

void CViewport::Draw()
{
	ImGui::Begin("viewport", nullptr, ImGuiWindowFlags_MenuBar);
	ImGui::Image((void*)m_pSRV, ImVec2(1020, 720));
	ImGui::End();
}
