#include "Settings.h"

void CSettings::Init(CCamera* pCCam , CLight* pCLight)
{
	m_CFps.Init();
	m_pCLight = pCLight;
	m_pCCam = pCCam;
}

void CSettings::Draw()
{
	if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_MenuBar))
	{ 
		m_CFps.Frame();
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS:%d", m_CFps.GetFps());
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "ENTTSInFrustum:%d", m_pCCam->GetENTTsInFrustum());
	
		ImGuiIO io = ImGui::GetIO();
		ImGui::Text("Mouse pos: %g, %g", io.MousePos.x, io.MousePos.y);
		ImGui::Separator();
		if (ImGui::TreeNode("Camera"))
		{   
			using namespace DirectX;
			static float DragFactor = 0.1f;
		
			XMVECTOR* curPosVec = m_pCCam->GetPosition();
			XMFLOAT3 posFloat;
			XMStoreFloat3(&posFloat, *curPosVec);

			ImGui::Text("Position");
			ImGui::SliderFloat("X", &posFloat.x, -100, 100);
			ImGui::SliderFloat("Y", &posFloat.y, -100, 100);
			ImGui::SliderFloat("Z", &posFloat.z, -100, 100);
			ImGui::PushID(0);
			if (ImGui::Button("Reset"))
			{
				m_pCCam->ResetTranslation();
			}
			ImGui::PopID();


			XMVECTOR* angleVec = m_pCCam->GetRotation();
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
				m_pCCam->ResetRotation();
			}
			ImGui::Separator();
			ImGui::PopID();
			ImGui::TreePop();
		}


		if (ImGui::TreeNode("Lighting"))
		{  

			ImGui::TreePop();
		}

		ImGui::End();
	}
}
