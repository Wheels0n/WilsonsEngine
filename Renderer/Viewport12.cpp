#include "Viewport12.h"
#include "BackBuffer.h"
#include "Engine.h"
#include "SSAOResourceManager.h"
namespace wilson
{
	Viewport12::Viewport12(std::shared_ptr<D3D12> pD3D12, std::shared_ptr<Scene12> pScene)
		:m_IsFocused(false), m_width(_VIEWPORT_WIDTH), m_height(_VIEWPORT_HEIGHT), m_left(0),m_top(0),
		m_pD3D12(pD3D12), m_pScene(pScene)
	{
		m_pCam = pD3D12->GetCam();
		m_pImporter = make_shared<Importer12>();
	}

	XMVECTOR Viewport12::CalEntityPos()
	{
		ImGuiIO io = ImGui::GetIO();
		UINT width = io.DisplaySize.x;
		UINT height = io.DisplaySize.y;
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
	BOOL     Viewport12::CheckRange(const int x, const int y)
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

	void Viewport12::Draw()
	{
		shared_ptr<BackBuffer>					pBackBuffer = m_pD3D12->GetBackBuffer();
		shared_ptr<SSAOResourceManager>pSSAOResourceManager = m_pD3D12->GetSSAOResourceManager();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		if (ImGui::Begin("viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
		{
			Resize();
			ImVec2 pos = ImGui::GetWindowPos();
			m_left = pos.x;
			m_top = pos.y;
			m_IsFocused = ImGui::IsWindowFocused();

			pBackBuffer->DrawViewportImage();
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payLoad;
				for (int i = 0; i < 5; ++i)
				{
					payLoad = ImGui::AcceptDragDropPayload(g_types[i]);
					if (payLoad != nullptr)
					{	
						int entityIdx = m_pScene->GetNumEntity();
						XMVECTOR pos = CalEntityPos();
						if (i < 2)
						{
							const wchar_t* path = reinterpret_cast<const wchar_t*>(payLoad->Data);
							m_pImporter->LoadObject(g_types[i], path, D3D12::GetDevice());
							shared_ptr<MeshGroup12> pObject = m_pImporter->GetMesh();

							vector<shared_ptr<Mesh12>>& pMeshes = pObject->GetMeshes();
							for (int i = 0; i < pMeshes.size(); ++i)
							{
								shared_ptr<Mesh12> pMesh = pMeshes[i];
								pMesh->SetParent(pObject);

								shared_ptr<TransformMatrices> pMatrices = pMesh->GetTransformMatrices();
								pMatrices->trMat = XMMatrixTranslationFromVector(pos);
								pMesh->SetMatrixDirtyBit(true);
							}
							int meshIdx = m_pScene->GetNumMesh();
							m_pScene->AddObject(pObject);
							m_pScene->AddMeshEntity(pObject, entityIdx, meshIdx);
							break;
						}
						else
						{
							shared_ptr<Light12> pLight = nullptr;
							string type;
							int lightIdx = _INVALID_INDEX;
							ComPtr<ID3D12Device> pDevice = D3D12::GetDevice();
							switch (i)
							{
							case 2:
								lightIdx = m_pScene->GetNumLight(eLIGHT_TYPE::DIR);
								pLight = make_shared<DirectionalLight12>(lightIdx, m_pCam);
								type = "DirectionalLight";
								break;
							case 3:
								lightIdx = m_pScene->GetNumLight(eLIGHT_TYPE::CUBE);
								pLight = make_shared<CubeLight12>(lightIdx);
								type = "CubeLight";
								break;
							case 4:
								lightIdx = m_pScene->GetNumLight(eLIGHT_TYPE::SPT);
								pLight = make_shared<SpotLight12>(lightIdx);
								type = "SpotLight";
							}
							shared_ptr<LightProperty> pProperty = pLight->GetProperty();
							XMStoreFloat3(&pProperty->position, pos);
							m_pScene->AddLight(pLight);
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
			pBackBuffer			->DrawGBufImage();
			pSSAOResourceManager->DrawSSAOBlurredImage();

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

	
	int   Viewport12::GetX()
	{
		return m_left;
	};
	int   Viewport12::GetY()
	{
		return m_top;
	};
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

}