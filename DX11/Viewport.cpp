#include "Viewport.h"

namespace wilson
{
	void Viewport::Init(CD3D11* pD3D11, Scene* pScene)
	{
		m_IsFocused = false;

		m_width = 1024;
		m_height = 720;
		m_left = 0;
		m_top = 0;

		m_pSwapChain = pD3D11->GetSwapChain();
		m_pSRV = pD3D11->GetRTT();
		m_pDevice = pD3D11->GetDevice();
		m_pD3D11 = pD3D11;
		m_pCam = pD3D11->GetCam();
		m_pScene = pScene;
	}

	void Viewport::Draw()
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
			float x = GetNDCX(io.MousePos.x) * width;
			float y = GetNDCY(io.MousePos.y) * height;

			ImGui::Image((void*)m_pSRV, ImVec2(m_width, m_height));
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payLoad;
				payLoad = ImGui::AcceptDragDropPayload("obj");
				if (payLoad != nullptr)
				{
					const wchar_t* path = (const wchar_t*)payLoad->Data;
					m_importer.LoadOBJ(path);
					Model* pModel = m_importer.GetModel();
					m_importer.LoadTex(pModel, L"./Assets/Textures/empty.png", m_pD3D11->GetDevice());
					m_importer.Clear();
					//
					XMMATRIX* pTrMat = pModel->GetTranslationMatrix();
					XMFLOAT4X4 trMat4;
					XMStoreFloat4x4(&trMat4, *pTrMat);

					XMVECTOR m_camPos = *(m_pCam->GetPosition());
					XMFLOAT4 camPos4;
					XMStoreFloat4(&camPos4, m_camPos);

					XMMATRIX projMat = *(m_pCam->GetProjectionMatrix());
					XMFLOAT4X4 projMat4;
					XMStoreFloat4x4(&projMat4, projMat);
					float ratio = width / (float)height;

					float dx = (x / (width * 0.5f) - 1.0f) / (projMat4._22 * ratio);
					float dy = (1.0f - y / (height * 0.5f)) / projMat4._22;
					float dz = camPos4.z + 6;

					XMVECTOR pPos = XMVectorSet(dx * dz, dy * dz, dz, 0.0f);

					XMMATRIX viewMat = *(m_pCam->GetViewMatrix());
					XMMATRIX invViewMat = XMMatrixInverse(nullptr, viewMat);
					pPos = XMVector4Transform(pPos, invViewMat);

					XMFLOAT4 pos4;
					XMStoreFloat4(&pos4, pPos);
					pos4.z = camPos4.z + 1;
					pPos = XMLoadFloat4(&pos4);
					XMMATRIX trMat = XMMatrixTranslationFromVector(pPos);
					*pTrMat = trMat;
					//
					m_pD3D11->AddModel(pModel, m_pDevice);
					m_pScene->AddEntity(pModel);

				}

				for (int i = 0; i < 3; ++i)
				{
					payLoad = ImGui::AcceptDragDropPayload(texFormats[i]);
					if (payLoad != nullptr)
					{
						const wchar_t* path = (const wchar_t*)payLoad->Data;

						//m_pCScene->Pick(GetNDCX(x) * width, GetNDCY(y) * height, width, height);
						Entity* pENTT = nullptr;
						pENTT = m_pScene->GetSelectedENTT();
						if (pENTT != nullptr)
						{
							m_importer.LoadTex(pENTT->GetModel(), path, m_pD3D11->GetDevice());
						}

					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::End();
		}
		ImGui::PopStyleVar(2);
	}
	void Viewport::Resize()
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

	bool Viewport::CheckRange(int x, int y)
	{
		//ImGui는 사용자 모니터 해상도를 기준으로 좌표 계산.
		if (m_IsFocused == false ||
			x<m_left ||
			y<m_top ||
			x>m_left + m_width ||
			y>m_top + m_height)
		{
			return false;
		}

		return true;
	}

	float Viewport::GetNDCX(int x)
	{
		int viewportX = m_left;
		int viewportWidth = m_width;

		return (float)(x - viewportX) / viewportWidth;
	}

	float Viewport::GetNDCY(int y)
	{
		int viewportY = m_top;
		int viewportHeight = m_height;

		return (float)(y - viewportY) / viewportHeight;
	}
}