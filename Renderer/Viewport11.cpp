
#include "Viewport11.h"

namespace wilson
{
	Viewport11::Viewport11(D3D11* const pD3D11, Scene11* const pScene)
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
		m_pImporter = std::make_unique<Importer11>(m_pDevice.Get());

	}

	Viewport11::~Viewport11()
	{

	}

	void Viewport11::Draw()
	{	
		m_pPostProcessSrv = m_pD3D11->GetPostProcessSrv();
		m_pSsaoBlurredSrv = m_pD3D11->GetSsaoBlurredSrv();
		for (int i = 0; i < static_cast<UINT>(eGbuf::cnt); ++i)
		{
			m_ppGbufferSrvs[i] = *(m_pD3D11->GetpGbufferSrvs()+i);
		}
		m_nGbuffer = m_pD3D11->GetNumGBuffer();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		if (ImGui::Begin("viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
		{
			Resize();
			ImVec2 pos = ImGui::GetWindowPos();
			m_left = pos.x;
			m_top = pos.y;
			m_IsFocused = ImGui::IsWindowFocused();

			ImGui::Image(reinterpret_cast<void*>(m_pPostProcessSrv.Get()), ImVec2(m_width, m_height));
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
							const wchar_t* path = reinterpret_cast<const wchar_t*>(payLoad->Data);
							m_pImporter->LoadObject(g_types[i], path, m_pDevice.Get());
							Object11* pObject = m_pImporter->GetpObject();

							std::vector<Mesh11*> pMeshes = pObject->GetMeshes();
							for (int i = 0; i < pMeshes.size(); ++i)
							{
								Mesh11* pMesh = pMeshes[i];
								XMMATRIX* pTrMat = pMesh->GetTranslationMatrix();
								XMMATRIX trMat = XMMatrixTranslationFromVector(pos);
								*pTrMat = trMat;
							}
							int meshdx = m_pD3D11->GetNumObject();
							m_pD3D11->AddObject(pObject);
							m_pScene->AddMeshEntity(pObject, newEntityIdx, meshdx);
							break;
						}
						else
						{
							Light11* pLight = nullptr;
							std::string type;
							int lightIdx = -1;
							ID3D11Device* pDevice= m_pD3D11->GetDevice();
							switch (i)
							{
							case 2:
								lightIdx = m_pD3D11->GetNumLight(eLIGHT_TYPE::DIR);
								pLight = new DirectionalLight11(pDevice, lightIdx, m_pCam);
								type = "DirectionalLight11";
								break;
							case 3:
								lightIdx = m_pD3D11->GetNumLight(eLIGHT_TYPE::CUBE);
								pLight = new CubeLight11(pDevice, lightIdx);
								type = "CubeLight11";
								break;
							case 4:
								lightIdx = m_pD3D11->GetNumLight(eLIGHT_TYPE::SPT);
								pLight = new SpotLight11(pDevice, lightIdx);
								type = "SpotLight11";
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
			for (int i = 0; i < m_nGbuffer; ++i)
			{
				ImGui::Image(reinterpret_cast<void*>(m_ppGbufferSrvs[i].Get()), ImVec2(m_width, m_height));
			}
			ImGui::Image(reinterpret_cast<void*>(m_pSsaoBlurredSrv.Get()), ImVec2(m_width, m_height));
			
		}
		ImGui::End();
	}
	void Viewport11::Resize()
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

	bool Viewport11::CheckRange(const int x, const int y)
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

	float Viewport11::GetNDCX(const int x)
	{
		int viewportX = m_left;
		int viewportWidth = m_width;

		return static_cast<float>(x - viewportX) / viewportWidth;
	}

	float Viewport11::GetNDCY(const int y)
	{
		int viewportY = m_top;
		int viewportHeight = m_height;

		return static_cast<float>(y - viewportY) / viewportHeight;
	}

	XMVECTOR Viewport11::CalEntityPos()
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
		float ratio = width / static_cast<float>(height);

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