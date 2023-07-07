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
			/*if (ImGui::TreeNode("Lighting"))
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
					ImGui::DragFloat("##X", &pos3.x, 1.0f, -10000.0f, 10000.0f);

					if (ImGui::Button("Y"))
					{
						pos3.y = 0.0f;
					}
					ImGui::SameLine();
					ImGui::DragFloat("##Y", &pos3.y, 1.0f, -10000.0f, 10000.0f);

					if (ImGui::Button("Z"))
					{
						pos3.z = 0.0f;
					}
					ImGui::SameLine();
					ImGui::DragFloat("##Z", &pos3.z, 1.0f, -10000.0f, 10000.0f);
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
			}*/

			
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