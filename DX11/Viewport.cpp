#include "Viewport.h"

void CViewport::Init(CD3D11* pCD3D11, CScene* pCScene)
{
	m_pSRV = pCD3D11->GetRTT();
	m_pDevice = pCD3D11->GetDevice();
	m_pCD3D11 = pCD3D11;
	m_pCScene = pCScene;
}

void CViewport::Draw()
{   
	ImGui::Begin("viewport", nullptr, ImGuiWindowFlags_MenuBar);
	ImGui::Image((void*)m_pSRV, ImVec2(1020, 720));
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("obj");
		if (payLoad != NULL)
		{
			const wchar_t* path = (const wchar_t*)payLoad->Data;
			m_CImporter.LoadOBJ(path);
			CModel* pModel = m_CImporter.GetModel();
			m_pCD3D11->AddModel(pModel, m_pDevice);

			std::wstring wStr(pModel->GetName());
			std::string str = std::string(wStr.begin(), wStr.end());
			m_pCScene->AddEntity(str ,pModel->GetWorldMatrix());
		}

		ImGui::EndDragDropTarget();
	}
	ImGui::End();
}
