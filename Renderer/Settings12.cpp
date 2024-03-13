#include<DirectXTex.h>

#include "Settings12.h"
#include "../ImGui/imgui.h"
namespace wilson
{
	const static float _ICON_SZ = 16.0f;
	const static float _PAD = 32.0f;
	Settings12::Settings12(D3D12* pD3D12)
	{
		//D3D12
		m_pDirLitIcon12Tex=nullptr;
		m_pCubeLitIcon12Tex=nullptr;
		m_pSpotLitIcon12Tex=nullptr;
		m_pDirLitIcon12UploadCB=nullptr;
		m_pPntLitIcon12UploadCB=nullptr;
		m_pSpotLitIcon12UploadCB = nullptr;

		m_FPS.Init();
		m_pD3D12 = pD3D12;
		m_pCam = m_pD3D12->GetCam();
		m_pFrustum = m_pD3D12->GetFrustum();
		m_pExposure = m_pD3D12->GetExposure();
		m_pHeightScale = m_pD3D12->GetHeightScale();
		m_pHeightOnOFF = m_pD3D12->GetHeighOnOFF();

		ID3D12GraphicsCommandList* pCommandlist = m_pD3D12->GetCommandList();
		ID3D12Device* pDevice = pD3D12->GetDevice();
		DescriptorHeapManager* pDescriptorHeapManager = m_pD3D12->GetDescriptorHeapManager();

		HRESULT hr;
		//Gen DirLightIcon
		{
			//Gen DirIconTex
			{
				//Gen IconTexFromFile 
				DirectX::ScratchImage image;
				hr = DirectX::LoadFromWICFile(L"./Assets/Icons/sun-color-icon.png", DirectX::WIC_FLAGS_NONE, nullptr, image);
				UINT8* pData = image.GetPixels();
				size_t rowPitch;
				size_t slidePitch;
				DirectX::TexMetadata metadata = image.GetMetadata();
				ComputePitch(metadata.format, metadata.width, metadata.height, rowPitch, slidePitch);

				D3D12_RESOURCE_DESC	texDesc = {};
				texDesc.Width = image.GetMetadata().width;
				texDesc.Height = image.GetMetadata().height;
				texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
				texDesc.Alignment = 0;
				texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				texDesc.Format = image.GetMetadata().format;
				texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				texDesc.DepthOrArraySize = 1;
				texDesc.MipLevels = 1;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;

				D3D12_HEAP_PROPERTIES heapProps = {};
				heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
				heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heapProps.CreationNodeMask = 1;
				heapProps.VisibleNodeMask = 1;
				hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
					nullptr, IID_PPV_ARGS(&m_pDirLitIcon12Tex));
				assert(SUCCEEDED(hr));
				m_pDirLitIcon12Tex->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Settings12::m_pDirLitIcon12Tex") - 1, "Settings12::m_pDirLitIcon12Tex");
				
				m_pD3D12->UploadTexThroughCB(texDesc, rowPitch, pData, m_pDirLitIcon12Tex, &m_pDirLitIcon12UploadCB, pCommandlist);

				//Gen SRV
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Format = texDesc.Format;
				srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

				D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
				pDevice->CreateShaderResourceView(m_pDirLitIcon12Tex, &srvDesc, cbvSrvCpuHandle);
				m_dirLitIcon12SRV = cbvSrvGpuHandle;
				pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
			}
		}

		//Gen CubeLightIcon
		{
			{
				//Gen IconTexFromFile 
				DirectX::ScratchImage image;
				hr = DirectX::LoadFromWICFile(L"./Assets/Icons/sun-color-icon.png", DirectX::WIC_FLAGS_NONE, nullptr, image);
				UINT8* pData = image.GetPixels();
				size_t rowPitch;
				size_t slidePitch;
				DirectX::TexMetadata metadata = image.GetMetadata();
				ComputePitch(metadata.format, metadata.width, metadata.height, rowPitch, slidePitch);

				D3D12_RESOURCE_DESC	texDesc = {};
				texDesc.Width = image.GetMetadata().width;
				texDesc.Height = image.GetMetadata().height;
				texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
				texDesc.Alignment = 0;
				texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				texDesc.Format = image.GetMetadata().format;
				texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				texDesc.DepthOrArraySize = 1;
				texDesc.MipLevels = 1;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;

				D3D12_HEAP_PROPERTIES heapProps = {};
				heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
				heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heapProps.CreationNodeMask = 1;
				heapProps.VisibleNodeMask = 1;
				hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
					nullptr, IID_PPV_ARGS(&m_pCubeLitIcon12Tex));
				assert(SUCCEEDED(hr));
				m_pCubeLitIcon12Tex->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Settings12::m_pCubeLitIcon12Tex") - 1, "Settings12::m_pCubeLitIcon12Tex");

				m_pD3D12->UploadTexThroughCB(texDesc, rowPitch, pData, m_pCubeLitIcon12Tex, &m_pPntLitIcon12UploadCB, pCommandlist);

				//Gen SRV
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Format = texDesc.Format;
				srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

				D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
				pDevice->CreateShaderResourceView(m_pCubeLitIcon12Tex, &srvDesc, cbvSrvCpuHandle);
				m_pntLitIcon12SRV = cbvSrvGpuHandle;
				pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
			}
		}

		//Gen SpotLightIcon
		{
			{
				//Gen IconTexFromFile 
				DirectX::ScratchImage image;
				hr = DirectX::LoadFromWICFile(L"./Assets/Icons/flashlight-icon.png", DirectX::WIC_FLAGS_NONE, nullptr, image);
				UINT8* pData = image.GetPixels();
				size_t rowPitch;
				size_t slidePitch;
				DirectX::TexMetadata metadata = image.GetMetadata();
				ComputePitch(metadata.format, metadata.width, metadata.height, rowPitch, slidePitch);

				D3D12_RESOURCE_DESC	texDesc = {};
				texDesc.Width = image.GetMetadata().width;
				texDesc.Height = image.GetMetadata().height;
				texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
				texDesc.Alignment = 0;
				texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				texDesc.Format = image.GetMetadata().format;;
				texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				texDesc.DepthOrArraySize = 1;
				texDesc.MipLevels = 1;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;

				D3D12_HEAP_PROPERTIES heapProps = {};
				heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
				heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heapProps.CreationNodeMask = 1;
				heapProps.VisibleNodeMask = 1;
				hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
					nullptr, IID_PPV_ARGS(&m_pSpotLitIcon12Tex));
				assert(SUCCEEDED(hr));
				m_pSpotLitIcon12Tex->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Settings12::m_pSpotLitIcon12Tex") - 1, "Settings12::m_pSpotLitIcon12Tex");

				m_pD3D12->UploadTexThroughCB(texDesc, rowPitch, pData, m_pSpotLitIcon12Tex, &m_pSpotLitIcon12UploadCB, pCommandlist);

				//Gen SRV
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Format = texDesc.Format;
				srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


				D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
				pDevice->CreateShaderResourceView(m_pSpotLitIcon12Tex, &srvDesc, cbvSrvCpuHandle);
				m_spotLitIcon12SRV = cbvSrvGpuHandle;
				pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
			}
		}

	}

	void Settings12::Draw()
	{
		if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_MenuBar))
		{
			m_FPS.Frame();
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS:%d", m_FPS.GetFps());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "TotalENTTs:%d", m_pFrustum->GetENTTsInTotal());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "ENTTSInFrustum:%d", m_pFrustum->GetENTTsInFrustum());
			ImGui::Separator();

			ImGuiIO io = ImGui::GetIO();
			ImGui::Text("Mouse Pos: %g, %g", io.MousePos.x, io.MousePos.y);
			ImGui::Separator();
			if (ImGui::TreeNode("Camera"))
			{
				using namespace DirectX;
				static float DragFactor = 0.1f;

				float* curSpeed = m_pCam->GetTRSpeed();
				float* curNearZ = m_pCam->GetNearZ();
				float* curFarZ = m_pCam->GetFarZ();

				ImGui::DragFloat("Speed", curSpeed, 0.1f, 10.0f);
				ImGui::DragFloat("NearZ", curNearZ, 0.001f, 100.f);
				ImGui::DragFloat("FarZ", curFarZ, 100.f, 10000.f);

				XMVECTOR* curPosVec = m_pCam->GetPosition();
				XMFLOAT3 posFloat;
				XMStoreFloat3(&posFloat, *curPosVec);

				ImGui::Text("Position");
				ImGui::DragFloat("X", &posFloat.x, -100, 100);
				ImGui::DragFloat("Y", &posFloat.y, -100, 100);
				ImGui::DragFloat("Z", &posFloat.z, -100, 100);
				*curPosVec = XMLoadFloat3(&posFloat);
				ImGui::PushID(0);
				if (ImGui::Button("Reset"))
				{
					m_pCam->ResetTranslation();
				}

				ImGui::PopID();

				XMVECTOR* angleVec = m_pCam->GetRotation();
				XMFLOAT3 angleFloat;
				XMStoreFloat3(&angleFloat, *angleVec);
				//Rad to Degree
				//angleFloat.x = 57.2958f;
				//angleFloat.y = 57.2958f;

				ImGui::Text("Rotation");
				ImGui::DragFloat("Pitch", &angleFloat.x, 0.01f, -180, 180);
				ImGui::DragFloat("Yaw", &angleFloat.y, 0.01f, -360, 360);
				//Degree to Rad
				//angleFloat.x /= 57.2958f;
				//angleFloat.y /= 57.2958f;
				*angleVec = XMLoadFloat3(&angleFloat);
				ImGui::PushID(1);
				if (ImGui::Button("Reset"))
				{
					m_pCam->ResetRotation();
				}

				if (m_prevPos.x != posFloat.x || m_prevPos.y != posFloat.y || m_prevPos.z != posFloat.z ||
					angleFloat.x != m_prevAngleFloat.x || angleFloat.y != m_prevAngleFloat.y ||
					m_prevNearZ != *curNearZ || m_prevFarZ != *curFarZ)
				{
					m_pCam->Update();
					delete m_pFrustum;
					m_pFrustum = new Frustum12(m_pCam);
					m_pD3D12->SetNewFrustum(m_pFrustum);
				}
				m_prevNearZ = *curNearZ;
				m_prevFarZ = *curFarZ;
				m_prevPos = posFloat;
				m_prevAngleFloat = angleFloat;

				ImGui::Separator();
				ImGui::PopID();

				ImGui::TreePop();
			}

		}
		ImGui::End();

		if (ImGui::Begin("Light"))
		{
			ImGui::Image((ImTextureID)m_dirLitIcon12SRV.ptr, ImVec2(_ICON_SZ, _ICON_SZ));
			ImGui::SameLine(_PAD);
			ImGui::Text("DirectionalLight");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("dir", nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			ImGui::Image((ImTextureID)m_pntLitIcon12SRV.ptr, ImVec2(_ICON_SZ, _ICON_SZ));
			ImGui::SameLine(_PAD);
			ImGui::Text("CubeLight");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("pnt", nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			ImGui::Image((ImTextureID)m_spotLitIcon12SRV.ptr, ImVec2(_ICON_SZ, _ICON_SZ));
			ImGui::SameLine(_PAD);
			ImGui::Text("SpotLight");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("spt", nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}


		}
		ImGui::End();
		if (ImGui::Begin("ON/OFF"))
		{
			if (ImGui::Button("ToggleParallaxMapping"))
			{
				*m_pHeightOnOFF = !(*m_pHeightOnOFF);
			}
		}
		ImGui::End();
		if (ImGui::Begin("Scale"))
		{
			ImGui::DragFloat("Exposure", m_pExposure, 0.01f, 0.1f, 5.0f);
			ImGui::DragFloat("HeightScale", m_pHeightScale, 0.0001f, 0.0001f, 5.0f, "%.4f");

		}
		ImGui::End();
	}

	Settings12::~Settings12()
	{

		if (m_pDirLitIcon12Tex != nullptr)
		{
			m_pDirLitIcon12Tex->Release();
			m_pDirLitIcon12Tex = nullptr;
		}
		
		if (m_pCubeLitIcon12Tex != nullptr)
		{
			m_pCubeLitIcon12Tex->Release();
			m_pCubeLitIcon12Tex = nullptr;
		}

		if (m_pSpotLitIcon12Tex != nullptr)
		{
			m_pSpotLitIcon12Tex->Release();
			m_pSpotLitIcon12Tex = nullptr;
		}
		
		if (m_pDirLitIcon12UploadCB != nullptr)
		{
			m_pDirLitIcon12UploadCB->Release();
			m_pDirLitIcon12UploadCB = nullptr;	
		}

		if (m_pPntLitIcon12UploadCB != nullptr)
		{
			m_pPntLitIcon12UploadCB->Release();
			m_pPntLitIcon12UploadCB = nullptr;
		}

		if (m_pSpotLitIcon12UploadCB != nullptr)
		{
			m_pSpotLitIcon12UploadCB->Release();
			m_pSpotLitIcon12UploadCB = nullptr;
		}

	}
}