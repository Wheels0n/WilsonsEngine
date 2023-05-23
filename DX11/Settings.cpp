#include "Settings.h"

namespace wilson
{
	void Settings::Init(Camera* pCCam, Light* pCLight)
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
				if(ImGui::TreeNode("DirectionalLight"))
				{  
					DirectionalLight dirLight = m_pLight->GetDirLight();
					
						DirectX::XMFLOAT3 pos3 = dirLight.position;
						ImGui::Text("Position");
						if (ImGui::Button("X"))
						{
							pos3.x = 0.0f;
						}
						ImGui::SameLine();
						ImGui::DragFloat("##X", &pos3.x, 1.0f, -1000.0f, 1000.0f);

						if (ImGui::Button("Y"))
						{
							pos3.y = 0.0f;
						}
						ImGui::SameLine();
						ImGui::DragFloat("##Y", &pos3.y, 1.0f, -1000.0f, 1000.0f);

						if (ImGui::Button("Z"))
						{
							pos3.z = 0.0f;
						}
						ImGui::SameLine();
						ImGui::DragFloat("##Z", &pos3.z, 1.0f, -1000.0f, 1000.0f);
						dirLight.position = pos3;

						DirectX::XMFLOAT4 ambient4;
						DirectX::XMStoreFloat4(&ambient4, dirLight.ambient);
						float ambient[4] = { ambient4.x, ambient4.y, ambient4.z, ambient4.w };
						ImGui::SliderFloat4("Ambient", ambient, 0.0f, 1.0f);
						ambient4 = DirectX::XMFLOAT4(ambient[0], ambient[1], ambient[2], ambient[3]);
						dirLight.ambient = DirectX::XMLoadFloat4(&ambient4);

						DirectX::XMFLOAT4 diffuse4;
						DirectX::XMStoreFloat4(&diffuse4, dirLight.diffuse);
						float diffuse[4] = { diffuse4.x, diffuse4.y, diffuse4.z, diffuse4.w };
						ImGui::SliderFloat4("Diffuse", diffuse, 0.0f, 1.0f);
						diffuse4 = DirectX::XMFLOAT4(diffuse[0], diffuse[1], diffuse[2], diffuse[3]);
						dirLight.diffuse = DirectX::XMLoadFloat4(&diffuse4);

						DirectX::XMFLOAT4 specular4;
						DirectX::XMStoreFloat4(&specular4, dirLight.specular);
						float specular[4] = { specular4.x, specular4.y, specular4.z, specular4.w };
						ImGui::SliderFloat4( "Specular", specular, 0.0f, 1.0f );
						specular4 = DirectX::XMFLOAT4(specular[0], specular[1], specular[2], specular[3]);
						dirLight.specular = DirectX::XMLoadFloat4(&specular4);

					m_pLight->SetDirLight(dirLight);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("PointLight"))
				{   
					PointLight pointLight = m_pLight->GetPointLight();
					DirectX::XMFLOAT3 pos3 = pointLight.position;
					ImGui::Text("Position");
					if (ImGui::Button("X"))
					{
						pos3.x = 0.0f;
					}
					ImGui::SameLine();
					ImGui::DragFloat("##X", &pos3.x, 0.01f, 0.0f, 1.0f);

					if (ImGui::Button("Y"))
					{
						pos3.y = 0.0f;
					}
					ImGui::SameLine();
					ImGui::DragFloat("##Y", &pos3.y, 0.01f, 0.0f, 1.0f);

					if (ImGui::Button("Z"))
					{
						pos3.z = 0.0f;
					}
					ImGui::SameLine();
					ImGui::DragFloat("##Z", &pos3.z, 0.01f, 0.0f, 1.0f);
					pointLight.position = pos3;

					DirectX::XMFLOAT4 ambient4;
					DirectX::XMStoreFloat4(&ambient4, pointLight.ambient);
					float ambient[4] = { ambient4.x, ambient4.y, ambient4.z, ambient4.w };
					ImGui::SliderFloat4("Ambient", ambient, 0.0f, 1.0f);
					ambient4 = DirectX::XMFLOAT4(ambient[0], ambient[1], ambient[2], ambient[3]);
					pointLight.ambient = DirectX::XMLoadFloat4(&ambient4);

					DirectX::XMFLOAT4 diffuse4;
					DirectX::XMStoreFloat4(&diffuse4, pointLight.diffuse);
					float diffuse[4] = { diffuse4.x, diffuse4.y, diffuse4.z, diffuse4.w };
					ImGui::SliderFloat4("Diffuse", diffuse, 0.0f, 1.0f);
					diffuse4 = DirectX::XMFLOAT4(diffuse[0], diffuse[1], diffuse[2], diffuse[3]);
					pointLight.diffuse = DirectX::XMLoadFloat4(&diffuse4);

					DirectX::XMFLOAT4 specular4;
					DirectX::XMStoreFloat4(&specular4, pointLight.specular);
					float specular[4] = { specular4.x, specular4.y, specular4.z, specular4.w };
					ImGui::SliderFloat4("Specular", specular, 0.0f, 1.0f);
					specular4 = DirectX::XMFLOAT4(specular[0], specular[1], specular[2], specular[3]);
					pointLight.specular = DirectX::XMLoadFloat4(&specular4);

					DirectX::XMFLOAT3 attenuation3 = pointLight.attenuation;
					float attenuation[3] = { attenuation3.x, attenuation3.y, attenuation3.z };
					ImGui::SliderFloat3("Attenuation", attenuation, 0.0f, 1.0f);
					pointLight.attenuation = DirectX::XMFLOAT3(attenuation[0], attenuation[1], attenuation[2]);

					float range = pointLight.range;
					if (ImGui::Button("Range"))
					{
						range = 0.0f;
					}
					ImGui::SameLine();
					ImGui::DragFloat("##Range", &range, 0.1f);
					pointLight.range = range;

					m_pLight->SetPointLight(pointLight);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("SpotLight"))
				{   
					SpotLight spotLight = m_pLight->GetSpotLight();

					DirectX::XMFLOAT3 dir3 = spotLight.direction;
					ImGui::Text("Direction");
					if (ImGui::Button("X"))
					{
						dir3.x = 0.0f;
					}
					ImGui::SameLine();
					ImGui::DragFloat("##X", &dir3.x, 0.1f);

					if (ImGui::Button("Y"))
					{
						dir3.y = 0.0f;
					}
					ImGui::SameLine();
					ImGui::DragFloat("##Y", &dir3.y, 0.1f);

					if (ImGui::Button("Z"))
					{
						dir3.z = 0.0f;
					}
					ImGui::SameLine();
					ImGui::DragFloat("##Z", &dir3.z, 0.1f);
					spotLight.direction = dir3;

					DirectX::XMFLOAT3 pos3 = spotLight.position;
					ImGui::Text("Position");
					if (ImGui::Button("X"))
					{
						pos3.x = 0.0f;
					}
					ImGui::SameLine();
					ImGui::DragFloat("###X", &pos3.x, 0.1f);

					if (ImGui::Button("Y"))
					{
						pos3.y = 0.0f;
					}
					ImGui::SameLine();
					ImGui::DragFloat("###Y", &pos3.y, 0.1f);

					if (ImGui::Button("Z"))
					{
						pos3.z = 0.0f;
					}
					ImGui::SameLine();
					ImGui::DragFloat("###Z", &pos3.z, 0.1f);
					spotLight.position = pos3;

					DirectX::XMFLOAT4 ambient4;
					DirectX::XMStoreFloat4(&ambient4, spotLight.ambient);
					float ambient[4] = { ambient4.x, ambient4.y, ambient4.z, ambient4.w };
					ImGui::SliderFloat4("Ambient", ambient, 0.0f, 1.0f);
					ambient4 = DirectX::XMFLOAT4(ambient[0], ambient[1], ambient[2], ambient[3]);
					spotLight.ambient = DirectX::XMLoadFloat4(&ambient4);

					DirectX::XMFLOAT4 diffuse4;
					DirectX::XMStoreFloat4(&diffuse4, spotLight.diffuse);
					float diffuse[4] = { diffuse4.x, diffuse4.y, diffuse4.z, diffuse4.w };
					ImGui::SliderFloat4("Diffuse", diffuse, 0.0f, 1.0f);
					diffuse4 = DirectX::XMFLOAT4(diffuse[0], diffuse[1], diffuse[2], diffuse[3]);
					spotLight.diffuse = DirectX::XMLoadFloat4(&diffuse4);

					DirectX::XMFLOAT4 specular4;
					DirectX::XMStoreFloat4(&specular4, spotLight.specular);
					float specular[4] = { specular4.x, specular4.y, specular4.z, specular4.w };
					ImGui::SliderFloat4("Specular", specular, 0.0f, 1.0f);
					specular4 = DirectX::XMFLOAT4(specular[0], specular[1], specular[2], specular[3]);
					spotLight.specular = DirectX::XMLoadFloat4(&specular4);

					DirectX::XMFLOAT3 attenuation3 = spotLight.attenuation;
					float attenuation[3] = { attenuation3.x, attenuation3.y, attenuation3.z };
					ImGui::SliderFloat3("Attenuation", attenuation, 0.0f, 1.0f);
					spotLight.attenuation = DirectX::XMFLOAT3(attenuation[0], attenuation[1], attenuation[2]);

					float range = spotLight.range;
					if (ImGui::Button("Range"))
					{
						range = 0.0f;
					}
					ImGui::SameLine();
					ImGui::DragFloat("##Range", &range, 1.0f);
					spotLight.range = range;

					float spot = spotLight.spot;
					if (ImGui::Button("Spot"))
					{
						spot = 0.0f;
					}
					ImGui::SameLine();
					ImGui::DragFloat("##Spot", &spot, 1.0f);
					spotLight.spot = spot;

					m_pLight->SetSpotLight(spotLight);
					ImGui::TreePop();
				}

				ImGui::TreePop();
			}

			ImGui::End();
		}
	}
}