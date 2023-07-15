#include "Settings.h"
#include<D3DX11tex.h>
namespace wilson
{	
	const static float ICON_SZ = 16.0f;
	const static float SPACING = 32.0f;
	void Settings::Init(ID3D11Device* pDevice, Camera* pCam)
	{
		m_FPS.Init();
		m_pCam = pCam;

		D3DX11CreateShaderResourceViewFromFileW(pDevice, L"./Assets/Icons/sun-color-icon.png", nullptr, nullptr, &m_icons[0], nullptr);
		D3DX11CreateShaderResourceViewFromFileW(pDevice, L"./Assets/Icons/light-bulb-color-icon.png", nullptr, nullptr, &m_icons[1], nullptr);
		D3DX11CreateShaderResourceViewFromFileW(pDevice, L"./Assets/Icons/flashlight-icon.png", nullptr, nullptr, &m_icons[2], nullptr);
	}

	void Settings::Draw()
	{
		if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_MenuBar))
		{
			m_FPS.Frame();
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS:%d", m_FPS.GetFps());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "ENTTSInFrustum:%d", m_pCam->GetENTTsInFrustum());

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
				ImGui::SliderFloat("Speed", curSpeed, 0.1f, 10.0f);
				ImGui::SliderFloat("NearZ", curNearZ, 0.001f, 100.f);
				ImGui::SliderFloat("FarZ", curFarZ,   100.f, 10000.f);

				XMVECTOR* curPosVec = m_pCam->GetPosition();
				XMFLOAT3 posFloat;
				XMStoreFloat3(&posFloat, *curPosVec);

				ImGui::Text("Position");
				ImGui::SliderFloat("X", &posFloat.x, -100, 100);
				ImGui::SliderFloat("Y", &posFloat.y, -100, 100);
				ImGui::SliderFloat("Z", &posFloat.z, -100, 100);
				ImGui::PushID(0);
				if (ImGui::Button("Reset"))
				{
					m_pCam->ResetTranslation();
				}
				ImGui::PopID();

				XMVECTOR* angleVec = m_pCam->GetRotation();
				XMFLOAT3 angleFloat;
				XMStoreFloat3(&angleFloat, *angleVec);
				angleFloat.x *= 57.2958f;
				angleFloat.y *= 57.2958f;

				ImGui::Text("Rotation");
				ImGui::SliderFloat("Pitch", &angleFloat.x, -180, 180);
				ImGui::SliderFloat("Yaw", &angleFloat.y, -360, 360);
				ImGui::PushID(1);
				if (ImGui::Button("Reset"))
				{
					m_pCam->ResetRotation();
				}
				ImGui::Separator();
				ImGui::PopID();
				ImGui::TreePop();
			}
			
		}
		ImGui::End();

		if (ImGui::Begin("Light"))
		{ 
			ImGui::Image(m_icons[0], ImVec2(ICON_SZ, ICON_SZ));
			ImGui::SameLine(SPACING);
			ImGui::Text("DirectionalLight");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{	
				ImGui::SetDragDropPayload("dir",nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			ImGui::Image(m_icons[1], ImVec2(ICON_SZ, ICON_SZ));
			ImGui::SameLine(SPACING);
			ImGui::Text("PointLight");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("pnt", nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			ImGui::Image(m_icons[2], ImVec2(ICON_SZ, ICON_SZ));
			ImGui::SameLine(SPACING);
			ImGui::Text("SpotLight");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("spt", nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			
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