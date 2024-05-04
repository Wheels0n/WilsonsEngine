#include<DirectXTex.h>

#include "Settings12.h"
#include "../ImGui/imgui.h"
namespace wilson
{
	const static float _ICON_SZ = 16.0f;
	const static float _PAD = 32.0f;
	Settings12::Settings12(D3D12*const pD3D12)
	{
		//D3D12
		m_FPS.Init();
		m_pD3D12 = pD3D12;
		m_pCam = m_pD3D12->GetCam();
		m_pExposure = m_pD3D12->GetExposure();
		m_pHeightScale = m_pD3D12->GetpHeightScale();
		m_pbHeightOn = m_pD3D12->GetpbHeightOn();
		m_pSsaoRadius = m_pD3D12->GetSsaoRadius();
		m_pSsaoBias = m_pD3D12->GetSsaoBias();
		m_pnSsaoSample = m_pD3D12->GetNumSsaoSample();

		ID3D12GraphicsCommandList* pCommandlist = m_pD3D12->GetCommandList();
		ID3D12Device* pDevice = pD3D12->GetDevice();
		HeapManager* pHeapManager = m_pD3D12->GetHeapManager();

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

				pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
					m_pDirLitIconTex.GetAddressOf(), pDevice);
				m_pDirLitIconTex->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Settings12::m_pDirLitIconTex") - 1, "Settings12::m_pDirLitIconTex");
				
				m_pD3D12->UploadTexThroughCB(texDesc, rowPitch, pData, m_pDirLitIconTex.Get(), m_pDirLitIconUploadCb.GetAddressOf(), pCommandlist);

				//Gen SRV
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Format = texDesc.Format;
				srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

				m_dirLitIconSrv = pHeapManager->GetSrv(srvDesc, m_pDirLitIconTex.Get(), pDevice);
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

				pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 
					m_pCubeLitIconTex.GetAddressOf(), pDevice);
				m_pCubeLitIconTex->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Settings12::m_pCubeLitIconTex") - 1, "Settings12::m_pCubeLitIconTex");

				m_pD3D12->UploadTexThroughCB(texDesc, rowPitch, pData, m_pCubeLitIconTex.Get(), m_pCubeLitIconUploadCb.GetAddressOf(), pCommandlist);

				//Gen SRV
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Format = texDesc.Format;
				srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

				m_cubeLitIconSrv = pHeapManager->GetSrv(srvDesc, m_pCubeLitIconTex.Get(), pDevice);
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

				pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 
					m_pSpotLitIconTex.GetAddressOf(), pDevice);
				m_pSpotLitIconTex->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Settings12::m_pSpotLitIconTex") - 1, "Settings12::m_pSpotLitIconTex");

				m_pD3D12->UploadTexThroughCB(texDesc, rowPitch, pData, m_pSpotLitIconTex.Get(), m_pSpotLitIconUploadCb.GetAddressOf(), pCommandlist);

				//Gen SRV
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Format = texDesc.Format;
				srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

				m_spotLitIconSrv = pHeapManager->GetSrv(srvDesc, m_pSpotLitIconTex.Get(), pDevice);
			}
		}

	}

	void Settings12::Draw()
	{
		if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_MenuBar))
		{
			Frustum12* pFrustum = m_pCam->GetFrustum();
			m_FPS.Frame();
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS:%d", m_FPS.GetFps());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "DrawCall:%d", m_pD3D12->GetNumDrawCall());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "InScene: %d", pFrustum->GetSubMeshesInScene());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "InFrustum: %d", pFrustum->GetSubMeshesInFrustum());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "HiZPassed: %d", m_pD3D12->GetNumSubMeshHiZPassed());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "NotOccluded: %d", m_pD3D12->GetNumSubMeshNotOccluded());
			ImGui::Separator();

			ImGuiIO io = ImGui::GetIO();
			ImGui::Text("Mouse Pos: %g, %g", io.MousePos.x, io.MousePos.y);
			ImGui::Separator();
			if (ImGui::TreeNode("Camera"))
			{
				using namespace DirectX;
				static float DragFactor = 0.1f;

				float* curSpeed = m_pCam->GetTrSpeed();
				float* curNearZ = m_pCam->GetNearZ();
				float* curFarZ = m_pCam->GetFarZ();

				ImGui::DragFloat("Speed", curSpeed, 0.1f, 10.0f);
				ImGui::DragFloat("NearZ", curNearZ, 0.001f, 100.f);
				ImGui::DragFloat("FarZ", curFarZ, 100.f, 10000.f);

				XMVECTOR* curPosVec = m_pCam->GetPosition();
				XMFLOAT4 posFloat;
				XMStoreFloat4(&posFloat, *curPosVec);

				ImGui::Text("Position");
				ImGui::DragFloat("X", &posFloat.x, -100, 100);
				ImGui::DragFloat("Y", &posFloat.y, -100, 100);
				ImGui::DragFloat("Z", &posFloat.z, -100, 100);
				posFloat.w=1.0f;
				*curPosVec = XMLoadFloat4(&posFloat);
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
					m_pCam->SetDirty(true);
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
			ImGui::Image((ImTextureID)m_dirLitIconSrv.ptr, ImVec2(_ICON_SZ, _ICON_SZ));
			ImGui::SameLine(_PAD);
			ImGui::Text("DirectionalLight");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("dir", nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			ImGui::Image((ImTextureID)m_cubeLitIconSrv.ptr, ImVec2(_ICON_SZ, _ICON_SZ));
			ImGui::SameLine(_PAD);
			ImGui::Text("CubeLight");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("pnt", nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			ImGui::Image((ImTextureID)m_spotLitIconSrv.ptr, ImVec2(_ICON_SZ, _ICON_SZ));
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
			if (ImGui::Button("ParallaxMapping"))
			{
				*m_pbHeightOn = !(*m_pbHeightOn);
			}
		}
		ImGui::End();
		if (ImGui::Begin("Scale"))
		{
			ImGui::DragFloat("Exposure", m_pExposure, 0.01f, 0.1f, 5.0f);
			ImGui::DragFloat("HeightScale", m_pHeightScale, 0.0001f, 0.0001f, 5.0f, "%.4f");

		}
		ImGui::End();
		if (ImGui::Begin("SSAO"))
		{
			ImGui::DragInt("SampleCount", reinterpret_cast<INT*>(m_pnSsaoSample), 1, 1, 64);
			ImGui::DragFloat("Bias", m_pSsaoBias, 0.01f, 0.1f, 5.0f);
			ImGui::DragFloat("Radius", m_pSsaoRadius, 0.01f, 0.1f, 5.0f);
		}
		ImGui::End();
	}

	Settings12::~Settings12()
	{

		
	}
}