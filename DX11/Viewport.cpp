#include "Viewport.h"

void CViewport::Init(CD3D11* pCD3D11, CScene* pCScene)
{  
	width = 0;
	height = 0;
	left = 0;;
	top=0;

	m_pSRV = pCD3D11->GetRTT();
	m_pDevice = pCD3D11->GetDevice();
	m_pCD3D11 = pCD3D11;
	m_pCScene = pCScene;
}

void CViewport::Draw()
{   
	ImGui::Begin("viewport", nullptr, ImGuiWindowFlags_MenuBar);
	ImGui::Image((void*)m_pSRV, ImVec2(1020, 720));
	
	ImVec2 pos = ImGui::GetWindowPos();
	left = pos.x;
	top = pos.y;
	ImVec2 sz = ImGui::GetWindowSize();
	width = sz.x;
	height = sz.y;

	if (ImGui::BeginDragDropTarget())
	{  
		const ImGuiPayload* payLoad;
		payLoad = ImGui::AcceptDragDropPayload("obj");
		if (payLoad != nullptr)
		{ 
			const wchar_t* path = (const wchar_t*)payLoad->Data;
			m_CImporter.LoadOBJ(path);
			CModel* pModel = m_CImporter.GetModel();
			m_CImporter.LoadTex(pModel, L"./Assets/Textures/empty.png", m_pCD3D11->GetDevice());
			m_CImporter.Clear();

			m_pCD3D11->AddModel(pModel, m_pDevice);
			m_pCScene->AddEntity(pModel);
		}

		payLoad = ImGui::AcceptDragDropPayload("png");
		if (payLoad != nullptr)
		{
			const wchar_t* path = (const wchar_t*)payLoad->Data;
			ImGuiIO io= ImGui::GetIO();

			int width = m_pCD3D11->GetClientWidth();
			int height = m_pCD3D11->GetClientHeight();
			m_pCScene->Pick(io.MousePos.x, io.MousePos.y, width, height);
			CEntity* pENTT = m_pCScene->GetSelectedENTT();

			m_CImporter.LoadTex(pENTT->GetModel(), path, m_pCD3D11->GetDevice());
		}


		ImGui::EndDragDropTarget();
	}
	ImGui::End();
}
ImVec2 GetViewportPos()
{
	return 
}