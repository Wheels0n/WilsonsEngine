#include "Viewport12.h"

namespace wilson
{
	Viewport12::Viewport12(D3D12*const pD3D12, Scene12*const pScene)
	{
		m_IsFocused = false;

		m_width = _VIEWPORT_WIDTH;
		m_height = _VIEWPORT_HEIGHT;
		m_left = 0;
		m_top = 0;

		m_pD3D12 = pD3D12;
		m_pDevice = pD3D12->GetDevice();
		m_pCam = pD3D12->GetCam();
		m_pHeapManager = pD3D12->GetHeapManager();
		m_pScene = pScene;
		m_pImporter = std::make_unique<Importer12>(m_pD3D12);

	}

	Viewport12::~Viewport12()
	{
	}

	void Viewport12::Draw()
	{
		m_pPostProcessSrv = m_pD3D12->GetPostProcessSrv();
		m_pSceneDepthSrv = m_pD3D12->GetDepthSrv();
		m_pSsaoBlurredSrv = m_pD3D12->GetSsaoBlurredSrv();
		m_pGbufferSrvs = m_pD3D12->GetpGbufferSrvs();
		m_nGbuffer = m_pD3D12->GetNumGBuffer();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		if (ImGui::Begin("viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
		{
			Resize();
			ImVec2 pos = ImGui::GetWindowPos();
			m_left = pos.x;
			m_top = pos.y;
			m_IsFocused = ImGui::IsWindowFocused();

			ImGui::Image((ImTextureID)m_pPostProcessSrv->ptr, ImVec2(m_width, m_height));
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payLoad;
				for (int i = 0; i < 5; ++i)
				{
					payLoad = ImGui::AcceptDragDropPayload(g_types[i]);
					if (payLoad != nullptr)
					{	
						//엔티티와 별개로 모델과 광원의 인덱스를 따로 구성함에 유의
						int entityIdx = m_pScene->GetEntitySize();
						XMVECTOR pos = CalEntityPos();
						if (i < 2)
						{
							const wchar_t* path = reinterpret_cast<const wchar_t*>(payLoad->Data);
							m_pImporter->LoadObject(g_types[i], path, m_pDevice);
							Object12* pObject = m_pImporter->GetpObject();

							std::vector<std::unique_ptr<Mesh12>>& pMeshes = pObject->GetMeshes();
							for (int i = 0; i < pMeshes.size(); ++i)
							{
								Mesh12* pMesh = pMeshes[i].get();
								XMMATRIX* pTrMat = pMesh->GetTranslationMatrix();
								XMMATRIX trMat = XMMatrixTranslationFromVector(pos);
								*pTrMat = trMat;
							}
							int meshIdx = m_pD3D12->GetNumObject();
							m_pD3D12->AddObject(pObject);
							m_pScene->AddMeshEntity(pObject, entityIdx, meshIdx);
							break;
						}
						else
						{
							Light12* pLight = nullptr;
							std::string type;
							int lightIdx = -1;
							ID3D12Device* pDevice = m_pD3D12->GetDevice();
							switch (i)
							{
							case 2:
								lightIdx = m_pD3D12->GetNumLight(eLIGHT_TYPE::DIR);
								pLight = new DirectionalLight12(pDevice, m_pD3D12->GetCommandList(), m_pHeapManager, lightIdx, m_pCam);
								type = "DirectionalLight";
								break;
							case 3:
								lightIdx = m_pD3D12->GetNumLight(eLIGHT_TYPE::CUBE);
								pLight = new CubeLight12(pDevice, m_pD3D12->GetCommandList(), m_pHeapManager, lightIdx);
								type = "CubeLight";
								break;
							case 4:
								lightIdx = m_pD3D12->GetNumLight(eLIGHT_TYPE::SPT);
								pLight = new SpotLight12(pDevice, m_pD3D12->GetCommandList(), m_pHeapManager, lightIdx);
								type = "SpotLight";
							}
							DirectX::XMFLOAT3* pPos = pLight->GetPos();
							DirectX::XMStoreFloat3(pPos, pos);
							m_pD3D12->AddLight(pLight);
							m_pScene->AddLightEntity(pLight, type, entityIdx, lightIdx);
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
				ImGui::Image((ImTextureID)m_pGbufferSrvs[i].ptr, ImVec2(m_width, m_height));
			}
			ImGui::Image((ImTextureID)m_pSsaoBlurredSrv->ptr, ImVec2(m_width, m_height));
			ImGui::Image((ImTextureID)m_pSceneDepthSrv->ptr, ImVec2(m_width, m_height));

		}
		ImGui::End();
	}
	void Viewport12::Resize()
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

	bool Viewport12::CheckRange(const int x, const int y)
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

	float Viewport12::GetNDCX(const int x)
	{
		int viewportX = m_left;
		int viewportWidth = m_width;

		return (static_cast<float>(x - viewportX) / viewportWidth);
	}

	float Viewport12::GetNDCY(const int y)
	{
		int viewportY = m_top;
		int viewportHeight = m_height;

		return (static_cast<float>(y - viewportY) / viewportHeight);
	}

	XMVECTOR Viewport12::CalEntityPos()
	{
		ImGuiIO io = ImGui::GetIO();
		UINT width = m_pD3D12->GetClientWidth();
		UINT height = m_pD3D12->GetClientHeight();
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