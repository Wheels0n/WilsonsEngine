#include "Settings.h"
#include<D3DX11tex.h>
namespace wilson
{	
	const static float _ICON_SZ = 16.0f;
	const static float _PAD = 32.0f;
	void Settings::Init(D3D11* pD3D11)
	{
		m_FPS.Init();
		m_pD3D11 = pD3D11;
		m_pCam = pD3D11->GetCam();
		m_pFrustum = pD3D11->GetFrustum();
		m_pExposure = pD3D11->GetExposure();

		ID3D11Device* pDevice= pD3D11->GetDevice();
		D3DX11CreateShaderResourceViewFromFileW(pDevice, L"./Assets/Icons/sun-color-icon.png", nullptr, nullptr, &m_icons[0], nullptr);
		m_icons[0]->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Importer::m_icons[0]") - 1, "Importer::m_icons[0]");
		D3DX11CreateShaderResourceViewFromFileW(pDevice, L"./Assets/Icons/light-bulb-color-icon.png", nullptr, nullptr, &m_icons[1], nullptr);
		m_icons[1]->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Importer::m_icons[1]") - 1, "Importer::m_icons[1]");
		D3DX11CreateShaderResourceViewFromFileW(pDevice, L"./Assets/Icons/flashlight-icon.png", nullptr, nullptr, &m_icons[2], nullptr);
		m_icons[2]->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Importer::m_icons[2]") - 1, "Importer::m_icons[2]");
	}

	void Settings::Draw()
	{
		if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_MenuBar))
		{
			m_FPS.Frame();
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS:%d", m_FPS.GetFps());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "TotalENTTs:%d", m_pFrustum->GetENTTsInTotal());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "ENTTSInFrustum:%d", m_pFrustum->GetENTTsInFrustum());
			ImGui::Separator();
			if (ImGui::Button("Toggle AABGrid"))
			{
				m_pD3D11->ToggleAABBGrid();
			}

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
				angleFloat.x *= 57.2958f;
				angleFloat.y *= 57.2958f;

				ImGui::Text("Rotation");
				ImGui::DragFloat("Pitch", &angleFloat.x, 0.01f, -180, 180);
				ImGui::DragFloat("Yaw", &angleFloat.y, 0.01f,-360, 360);
				//Degree to Rad
				angleFloat.x /= 57.2958f;
				angleFloat.y /= 57.2958f;
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
					m_pFrustum->Init(m_pCam);
					
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
			ImGui::Image(m_icons[0], ImVec2(_ICON_SZ, _ICON_SZ));
			ImGui::SameLine(_PAD);
			ImGui::Text("DirectionalLight");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{	
				ImGui::SetDragDropPayload("dir",nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			ImGui::Image(m_icons[1], ImVec2(_ICON_SZ, _ICON_SZ));
			ImGui::SameLine(_PAD);
			ImGui::Text("PointLight");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("pnt", nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			ImGui::Image(m_icons[2], ImVec2(_ICON_SZ, _ICON_SZ));
			ImGui::SameLine(_PAD);
			ImGui::Text("SpotLight");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("spt", nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			
		}
		ImGui::End();
		
		if (ImGui::Begin("Exposure"))
		{
			ImGui::DragFloat("Exposure", m_pExposure, 0.01f, 0.1f, 5.0f);
		}
		ImGui::End();
	}
	Settings::~Settings()
	{
		for (int i = 0; i < 3; ++i)
		{
			m_icons[i]->Release();
			m_icons[i] = nullptr;
		}
	}
}