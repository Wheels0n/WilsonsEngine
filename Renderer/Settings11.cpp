#include "Settings11.h"
#include <DirectXTex.h>
#include "../ImGui/imgui.h"
namespace wilson
{	
	const static float _ICON_SZ = 16.0f;
	const static float _PAD = 32.0f;
	Settings11::Settings11(D3D11*const pD3D11)
	{
		m_FPS.Init();
		m_pD3D11 = pD3D11;
		m_pCam = pD3D11->GetCam();
		m_pFrustum = pD3D11->GetFrustum();
		m_pExposure = pD3D11->GetExposure();
		m_pHeightScale = pD3D11->GetpHeightScale();
		m_pbHeightOn = pD3D11->GetpbHeightOn();

		ID3D11Device* pDevice= pD3D11->GetDevice();

		HRESULT hr;
		DirectX::ScratchImage image;
		hr = DirectX::LoadFromWICFile(L"./Assets/Icons/sun-color-icon.png", DirectX::WIC_FLAGS_NONE, nullptr, image);
		hr = CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), m_icons[0].GetAddressOf());
		assert(SUCCEEDED(hr));
		m_icons[0]->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Settings11::SunIcon") - 1, "Settings11::SunIcon");


		hr = DirectX::LoadFromWICFile(L"./Assets/Icons/light-bulb-color-icon.png", DirectX::WIC_FLAGS_NONE, nullptr, image);
		hr = CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), m_icons[1].GetAddressOf());
		assert(SUCCEEDED(hr));
		m_icons[1]->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Settings11::LightBulbIcon") - 1, "Settings11::LightBulbIcon");

		hr = DirectX::LoadFromWICFile(L"./Assets/Icons/flashlight-icon.png", DirectX::WIC_FLAGS_NONE, nullptr, image);
		hr = CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), m_icons[2].GetAddressOf());
		assert(SUCCEEDED(hr));
		m_icons[2]->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Settings11::FlashLightIcon") - 1, "Settings11::FlashLightIcon");
	}

	void Settings11::Draw()
	{
		if (ImGui::Begin("Settings11", nullptr, ImGuiWindowFlags_MenuBar))
		{
			m_FPS.Frame();
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS:%d", m_FPS.GetFps());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "TotalENTTs:%d", m_pFrustum->GetSubMeshesInScene());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "ENTTSInFrustum:%d", m_pFrustum->GetSubMeshesInFrustum());
			ImGui::Separator();

			ImGuiIO io = ImGui::GetIO();
			ImGui::Text("Mouse Pos: %g, %g", io.MousePos.x, io.MousePos.y);
			ImGui::Separator();
			if (ImGui::TreeNode("Camera11"))
			{
				using namespace DirectX;
				static float DragFactor = 0.1f;

				float* curSpeed = m_pCam->GetTrSpeed();
				float* curNearZ = m_pCam->GetNearZ();
				float* curFarZ = m_pCam->GetFarZ();
				
				ImGui::DragFloat("Speed", curSpeed, 0.1f, 10.0f);
				ImGui::DragFloat("NearZ", curNearZ, 0.001f, 100.f);
				ImGui::DragFloat("FarZ", curFarZ,   100.f, 10000.f);

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
				ImGui::DragFloat("Yaw", &angleFloat.y, 0.01f,-360, 360);
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
					m_pFrustum=new Frustum11(m_pCam);
					m_pD3D11->SetNewFrustum(m_pFrustum);
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

		if (ImGui::Begin("Light11"))
		{ 
			ImGui::Image(m_icons[0].Get(), ImVec2(_ICON_SZ, _ICON_SZ));
			ImGui::SameLine(_PAD);
			ImGui::Text("DirectionalLight11");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{	
				ImGui::SetDragDropPayload("dir",nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			ImGui::Image(m_icons[1].Get(), ImVec2(_ICON_SZ, _ICON_SZ));
			ImGui::SameLine(_PAD);
			ImGui::Text("CubeLight11");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("pnt", nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			ImGui::Image(m_icons[2].Get(), ImVec2(_ICON_SZ, _ICON_SZ));
			ImGui::SameLine(_PAD);
			ImGui::Text("SpotLight11");
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
	}
	
	Settings11::~Settings11()
	{

	}
}