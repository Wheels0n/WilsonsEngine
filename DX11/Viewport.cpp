#include "Viewport.h"

void CViewport::Init(CD3D11* pCD3D11, CScene* pCScene)
{  
	m_IsFocused = false;

	m_width = 1024;
	m_height = 720;
	m_left = 0;
	m_top=0;

	m_pSwapChain = pCD3D11->GetSwapChain();
	m_pSRV = pCD3D11->GetRTT();
	m_pDevice = pCD3D11->GetDevice();
	m_pD3D11 = pCD3D11;
	m_pCam = pCD3D11->GetCam();
	m_pCScene = pCScene;
}

void CViewport::Draw()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	if (ImGui::Begin("viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
	{
		Resize();
		ImVec2 pos = ImGui::GetWindowPos();
		m_left = pos.x;
		m_top = pos.y;
		m_IsFocused = ImGui::IsWindowFocused();

		ImGuiIO io = ImGui::GetIO();
		int width = m_pD3D11->GetClientWidth();
		int height = m_pD3D11->GetClientHeight();
		int x = GetNDCX(io.MousePos.x ) * width;
		int y = GetNDCY(io.MousePos.y) * height;

		ImGui::Image((void*)m_pSRV, ImVec2(m_width, m_height));
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payLoad;
			payLoad = ImGui::AcceptDragDropPayload("obj");
			if (payLoad != nullptr)
			{
				const wchar_t* path = (const wchar_t*)payLoad->Data;
				m_CImporter.LoadOBJ(path);
				Model* pModel = m_CImporter.GetModel();
				m_CImporter.LoadTex(pModel, L"./Assets/Textures/empty.png", m_pD3D11->GetDevice());
				m_CImporter.Clear();
				//
				XMMATRIX* pTr = pModel->GetTranslationMatrix();
				XMFLOAT4X4 tr4;
				XMStoreFloat4x4(&tr4, *pTr);

				XMVECTOR m_camPos = *(m_pCam->GetPosition());
				XMFLOAT4 camPos4;
				XMStoreFloat4(&camPos4, m_camPos);

				XMMATRIX projectionMat = *(m_pCam->GetProjectionMatrix());
				XMFLOAT4X4 projectionMat4;
				XMStoreFloat4x4(&projectionMat4, projectionMat);
				float ratio = width / (float)height;

				float dx = (x / (width * 0.5f) - 1.0f) /(projectionMat4._22 * ratio);
				float dy = (1.0f - y / (height * 0.5f)) /projectionMat4._22;
				float dz = camPos4.z + 6;

				XMVECTOR pPos = XMVectorSet(dx * dz, dy * dz, dz, 0.0f);
			   
				XMMATRIX viewMat = *(m_pCam->GetViewMatrix());
				XMMATRIX invViewMat = XMMatrixInverse(nullptr, viewMat);
				pPos = XMVector4Transform(pPos, invViewMat);
				
				XMFLOAT4 pos4;
				XMStoreFloat4(&pos4, pPos);
				pos4.z = camPos4.z + 1;
				pPos=XMLoadFloat4(&pos4);
				XMMATRIX tr = XMMatrixTranslationFromVector(pPos);
				*pTr = tr;
				//
				m_pD3D11->AddModel(pModel, m_pDevice);
				m_pCScene->AddEntity(pModel);

				
			}

			for (int i = 0; i < 3; ++i)
			{   
				payLoad = ImGui::AcceptDragDropPayload(texFormats[i]);
				if (payLoad != nullptr)
				{
					const wchar_t* path = (const wchar_t*)payLoad->Data;

					//m_pCScene->Pick(GetNDCX(x) * width, GetNDCY(y) * height, width, height);
					CEntity* pENTT = nullptr;
					pENTT = m_pCScene->GetSelectedENTT();
					if (pENTT != nullptr)
					{
						m_CImporter.LoadTex(pENTT->GetModel(), path, m_pD3D11->GetDevice());
					}

				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::End();
	}
	ImGui::PopStyleVar(2);
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
	//ImGui는 사용자 모니터 해상도를 기준으로 좌표 계산.
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

float CViewport::GetNDCX(int x)
{  
	int viewportX = GetX();
	int viewportWidth = GetWidth();

	return (float)(x - viewportX) / viewportWidth;
}

float CViewport::GetNDCY(int y)
{
	int viewportY = GetY();
	int viewportHeight = GetHeight();

	return (float)(y - viewportY) / viewportHeight;
}

