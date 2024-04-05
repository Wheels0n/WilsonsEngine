
#include "Viewport.h"

namespace wilson
{
	Viewport::Viewport(D3D11* pD3D11, Scene* pScene)
	{
		m_IsFocused = false;

		m_width = 1024;
		m_height = 720;
		m_left = 0;
		m_top = 0;

		m_pD3D11 = pD3D11;
		m_pSwapChain = pD3D11->GetSwapChain();
		m_pDevice = pD3D11->GetDevice();
		m_pD3D11 = pD3D11;
		m_pCam = pD3D11->GetCam();
		m_pScene = pScene;
		m_pImporter = new Importer(m_pDevice);

	}

	Viewport::~Viewport()
	{
		if (m_pImporter != nullptr)
		{
			delete m_pImporter;
			m_pImporter = nullptr;
		}
	}

	void Viewport::Draw()
	{	
		m_pFinalSRV = m_pD3D11->GetFinalSRV();
		m_pSSAOBlurredSRV = m_pD3D11->GetSSAOBlurredSRV();
		m_pGbufferSRV = m_pD3D11->GetGbufferSRV();
		m_GbufferCount = m_pD3D11->GetGbufferCount();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		if (ImGui::Begin("viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
		{
			Resize();
			ImVec2 pos = ImGui::GetWindowPos();
			m_left = pos.x;
			m_top = pos.y;
			m_IsFocused = ImGui::IsWindowFocused();

			ImGui::Image((void*)m_pFinalSRV, ImVec2(m_width, m_height));
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payLoad;
				for (int i = 0; i < 5; ++i)
				{
					payLoad = ImGui::AcceptDragDropPayload(g_types[i]);
					if (payLoad != nullptr)
					{
						int newEntityIdx = m_pScene->GetEntitySize();
						XMVECTOR pos = CalEntityPos();
						if (i < 2)
						{
							const wchar_t* path = (const wchar_t*)payLoad->Data;
							m_pImporter->LoadObject(g_types[i], path, m_pDevice);
							ModelGroup* pModelGroup = m_pImporter->GetpObject();

							std::vector<Model*> pModels = pModelGroup->GetMeshes();
							for (int i = 0; i < pModels.size(); ++i)
							{
								Model* pModel = pModels[i];
								XMMATRIX* pTrMat = pModel->GetTranslationMatrix();
								XMMATRIX trMat = XMMatrixTranslationFromVector(pos);
								*pTrMat = trMat;
							}
							int modelIdx = m_pD3D11->GetObjectSize();
							m_pD3D11->AddObject(pModelGroup);
							m_pScene->AddModelEntity(pModelGroup, newEntityIdx, modelIdx);
							break;
						}
						else
						{
							Light* pLight = nullptr;
							std::string type;
							int lightIdx = -1;
							ID3D11Device* pDevice= m_pD3D11->GetDevice();
							switch (i)
							{
							case 2:
								lightIdx = m_pD3D11->GetLightSize(eLIGHT_TYPE::DIR);
								pLight = new DirectionalLight(pDevice, lightIdx, m_pCam);
								type = "DirectionalLight";
								break;
							case 3:
								lightIdx = m_pD3D11->GetLightSize(eLIGHT_TYPE::CUBE);
								pLight = new CubeLight(pDevice, lightIdx);
								type = "CubeLight";
								break;
							case 4:
								lightIdx = m_pD3D11->GetLightSize(eLIGHT_TYPE::SPT);
								pLight = new SpotLight(pDevice, lightIdx);
								type = "SpotLight";
							}
							DirectX::XMFLOAT3* pPos = pLight->GetPos();
							DirectX::XMStoreFloat3(pPos, pos);

							m_pD3D11->AddLight(pLight);
							m_pScene->AddLightEntity(pLight, type, newEntityIdx, lightIdx);
						}
					}
				}

				ImGui::EndDragDropTarget();
			}
			
		}
		ImGui::End();
		ImGui::PopStyleVar(2);
		if (ImGui::Begin("Deferred"))
		{
			for (int i = 0; i < m_GbufferCount; ++i)
			{
				ImGui::Image((void*)m_pGbufferSRV[i], ImVec2(m_width, m_height));
			}
			ImGui::Image((void*)m_pSSAOBlurredSRV, ImVec2(m_width, m_height));
			
		}
		ImGui::End();
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

	XMVECTOR Viewport::CalEntityPos()
	{	
		ImGuiIO io = ImGui::GetIO();
		int width = m_pD3D11->GetClientWidth();
		int height = m_pD3D11->GetClientHeight();
		float x = GetNDCX(io.MousePos.x) * width;
		float y = GetNDCY(io.MousePos.y) * height;

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

		XMVECTOR pos = XMVectorSet(dx * dz, dy * dz, dz, 0.0f);

		XMMATRIX viewMat = *(m_pCam->GetViewMatrix());
		XMMATRIX invViewMat = XMMatrixInverse(nullptr, viewMat);
		pos = XMVector4Transform(pos, invViewMat);

		XMFLOAT4 pos4;
		XMStoreFloat4(&pos4, pos);
		pos4.z = camPos4.z + 1;
		pos = XMLoadFloat4(&pos4);
		return pos;
	}

}