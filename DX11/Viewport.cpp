#include "Viewport.h"

void CViewport::Init(CD3D11* pCD3D11, CScene* pCScene)
{  
	m_IsFocused = false;

	m_width = 1024;
	m_height = 720;
	m_left = 0;;
	m_top=0;

	m_pSwapChain = pCD3D11->GetSwapChain();
	m_pSRV = pCD3D11->GetRTT();
	m_pDevice = pCD3D11->GetDevice();
	m_pCD3D11 = pCD3D11;
	m_pCScene = pCScene;
}

void CViewport::Draw()
{   
	ImGui::Begin("viewport", nullptr, ImGuiWindowFlags_MenuBar| ImGuiWindowFlags_NoScrollbar);
	Resize();
	ImVec2 pos = ImGui::GetWindowPos();
	m_left = pos.x;
	m_top = pos.y;
	m_IsFocused = ImGui::IsWindowFocused();

	ImGui::Image((void*)m_pSRV, ImVec2(m_width, m_height));

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
void CViewport::Resize()
{   
	ImVec2 sz = ImGui::GetWindowSize();
	int width = sz.x;
	int height = sz.y;
	if (m_width != width || m_height != height)
	{  
		m_width = width;
		m_height = height;
	}
}

bool CViewport::CheckRange(int x, int y)
{     
	if (m_IsFocused ==false||
		x<m_left ||
		y<m_top  ||
		x>m_left + m_width||
		y>m_top + m_height)
	{
		return false;
	}

	return true;
}

