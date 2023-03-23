#include "Settings.h"

namespace wilson
{
	void Settings::Init(Camera* pCCam, CLight* pCLight)
	{
		m_CFps.Init();
		m_pLight = pCLight;
		m_pCam = pCCam;
	}

	void Settings::Draw()
	{
		if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_MenuBar))
		{
			m_CFps.Frame();
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS:%d", m_CFps.GetFps());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "ENTTSInFrustum:%d", m_pCam->GetENTTsInFrustum());

			ImGuiIO io = ImGui::GetIO();
			ImGui::Text("Mouse Pos: %g, %g", io.MousePos.x, io.MousePos.y);
			ImGui::Separator();
			if (ImGui::TreeNode("Camera"))
			{
				using namespace DirectX;
				static float DragFactor = 0.1f;

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


			if (ImGui::TreeNode("Lighting"))
			{
				XMVECTOR* dir = m_pLight->GetDirection();
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
				m_pLight->SetDirection(XMLoadFloat4(&dir4));

				float* pow = m_pLight->GetSpecPow();
				ImGui::Text("Intensity/Color");
				if (ImGui::Button("Intensity"))
				{
					*pow = 0.0f;
				}
				ImGui::SameLine();
				ImGui::DragFloat("##Intensity", pow, 0.1f);
				m_pLight->SetSpecPow(*pow);

				XMVECTOR* pAmbi = m_pLight->GetAmbient();
				XMFLOAT4 ambi4;
				XMStoreFloat4(&ambi4, *pAmbi);
				float ambi[4] = { ambi4.x, ambi4.y, ambi4.z, ambi4.w };
				ImGui::SliderFloat4("Ambient", ambi, 0.0f, 1.0f);
				m_pLight->SetAmbient(XMVectorSet(ambi[0], ambi[1], ambi[2], ambi[3]));

				XMVECTOR* pDiff = m_pLight->GetDiffuse();
				XMFLOAT4 diff4;
				XMStoreFloat4(&diff4, *pDiff);
				float diff[4] = { diff4.x, diff4.y, diff4.z, diff4.w };
				ImGui::SliderFloat4("Diffuse", diff, 0.0f, 1.0f);
				m_pLight->SetDiffuse(XMVectorSet(diff[0], diff[1], diff[2], diff[3]));

				XMVECTOR* pSpec = m_pLight->GetSpecular();
				XMFLOAT4 spec4;
				XMStoreFloat4(&spec4, *pSpec);
				float spec[4] = { spec4.x, spec4.y, spec4.z, spec4.w };
				ImGui::SliderFloat4("Specular", spec, 0.0f, 1.0f);
				m_pLight->SetSpecular(XMVectorSet(spec[0], spec[1], spec[2], spec[3]));

				ImGui::TreePop();
			}

			ImGui::End();
		}
	}
}