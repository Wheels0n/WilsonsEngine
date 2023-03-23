#include "Settings.h"

void CSettings::Init(Camera* pCCam , CLight* pCLight)
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
			XMVECTOR* dir = m_pCLight->GetDirection();
		    XMFLOAT4 dir4;
			XMStoreFloat4(&dir4, *dir);
			ImGui::Text("Direction");
			if (ImGui::Button("X"))
			{
				dir4.x = 0.0f;
			}
			ImGui::SameLine();
			ImGui::DragFloat("##X", &dir4.x, 0.1f);

			if (ImGui::Button("Y"))
			{
				dir4.y = 0.0f;
			}
			ImGui::SameLine();
			ImGui::DragFloat("##Y", &dir4.y, 0.1f);

			if (ImGui::Button("Z"))
			{
				dir4.z = 0.0f;
			}
			ImGui::SameLine();
			ImGui::DragFloat("##Z", &dir4.z, 0.1f);
			m_pCLight->SetDirection(XMLoadFloat4(&dir4));

			float* pow = m_pCLight->GetSpecPow();
			ImGui::Text("Intensity/Color");
			if (ImGui::Button("Intensity"))
			{
				*pow= 0.0f;
			}
			ImGui::SameLine();
			ImGui::DragFloat("##Intensity", pow, 0.1f);
			m_pCLight->SetSpecPow(*pow);

			XMVECTOR* Ambi = m_pCLight->GetAmbient();
			XMFLOAT4 Ambi4;
			XMStoreFloat4(&Ambi4, *Ambi);
			float pAmbi[4] = { Ambi4.x, Ambi4.y, Ambi4.z, Ambi4.w };
			ImGui::SliderFloat4("Ambient", pAmbi, 0.0f, 1.0f);
			m_pCLight->SetAmbient(XMVectorSet(pAmbi[0], pAmbi[1], pAmbi[2], pAmbi[3]));

			XMVECTOR* diff = m_pCLight->GetDiffuse();
			XMFLOAT4 diff4;
			XMStoreFloat4(&diff4, *diff);
		    float pDiff[4] = { diff4.x, diff4.y, diff4.z, diff4.w };
			ImGui::SliderFloat4("Diffuse", pDiff, 0.0f, 1.0f);
			m_pCLight->SetDiffuse(XMVectorSet(pDiff[0], pDiff[1], pDiff[2], pDiff[3]));

			XMVECTOR* spec = m_pCLight->GetSpecular();
			XMFLOAT4 spec4;
			XMStoreFloat4(&spec4, *spec);
			float pSpec[4] = { spec4.x, spec4.y, spec4.z, spec4.w };
			ImGui::SliderFloat4("Specular", pSpec, 0.0f, 1.0f);
			m_pCLight->SetSpecular(XMVectorSet(pSpec[0], pSpec[1], pSpec[2], pSpec[3]));

			ImGui::TreePop();
		}

		ImGui::End();
	}
}
