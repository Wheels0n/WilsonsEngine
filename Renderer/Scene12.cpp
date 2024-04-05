#include <cmath>
#include <string>

#include "Scene12.h"

namespace wilson
{
	Scene12::Scene12(D3D12* pD3D12)
	{
		m_isObject = false;
		sceneHandler = this;

		m_pSelectedEntity = nullptr;
		m_pCam = nullptr;

		m_pD3D12 = pD3D12;
		m_pShadow = pD3D12->GetShadowMap();
	}
	Scene12::~Scene12()
	{
		for (int i = 0; i < m_entites.size(); ++i)
		{
			delete m_entites[i];
		}
		m_entites.clear();
		m_entites.shrink_to_fit();
	}

	void Scene12::AddModelEntity(Object* pModelGroup, UINT entityIdx, UINT modelIdx)
	{
		std::string name = pModelGroup->GetName();

		Entity12* ENTT = new Entity12(name, entityIdx, pModelGroup, modelIdx);
		m_entites.push_back(ENTT);
	}
	void Scene12::AddLightEntity(Light12* pLight, std::string type, UINT entityIdx, UINT lightIdx)
	{
		std::string name = type;

		Entity12* ENTT = new Entity12(type, entityIdx, pLight, lightIdx);
		m_entites.push_back(ENTT);
	}
	void Scene12::Draw()
	{
		const char* actions = "Remove";
		if (ImGui::Begin("Scene Hierarchy"))
		{
			if (ImGui::TreeNode(m_name.c_str()))
			{
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
				//i:Entitiy배열, j:모델 그룹 내 하위 모델 배열
				for (int i = 0; i < m_entites.size(); ++i)
				{
					int entityIdx = m_entites[i]->GetEntityIndex();
					if (m_entites[i]->isObject())
					{
						Object* pModelGroup = m_entites[i]->GetpObject();
						std::string groupName = pModelGroup->GetName();
						std::vector<Mesh*>& pModels = pModelGroup->GetMeshes();
						UINT modelGroupIdx = m_entites[i]->GetObjectIndex();
						if (ImGui::TreeNode(groupName.c_str()))
						{
							if (ImGui::Button(groupName.c_str()))
							{
								//모델이 아닌 모델그룹이므로 관련 변수들 초기화.
								UnselectModel();
								ImGui::OpenPopup("Edit");
							}
							if (ImGui::BeginPopup("Edit"))
							{
								ImGui::Text("Edit");
								ImGui::Separator();
								if (ImGui::Selectable(actions))
								{	
									RemoveObject(modelGroupIdx, i);
									
									//뒤에 있던 entity와 model의 인덱스를 감소
									for (int j = i; j < m_entites.size(); ++j)
									{
										if (m_entites[j]->isObject())
										{
											m_entites[j]->DecreaseObjectIndex();
											m_entites[j]->DecreaseEntityIndex();
										}
									}
								}
								ImGui::EndPopup();
							}
							for (int j = 0; j < pModels.size(); ++j)
							{
								ImGui::PushID(i);
								std::string modelName = pModels[j]->GetName();
								std::string popUpID = std::to_string(i);
								popUpID += '-';
								popUpID += std::to_string(j);
								if (ImGui::Button(modelName.c_str()))
								{
									m_isObject = true;
									m_pSelectedEntity = pModels[j];

									m_pD3D12->PickSubMesh(modelGroupIdx, j);
									m_popUpID = popUpID;
									ImGui::OpenPopup(m_popUpID.c_str());
								}
								if (m_popUpID == popUpID && ImGui::BeginPopup(m_popUpID.c_str()))
								{
									ImGui::Text("Edit");
									ImGui::Separator();
									if (ImGui::Selectable(actions))
									{
										RemoveSelectedModel(modelGroupIdx, j);
										if (!m_pD3D12->GetNumMesh(modelGroupIdx))
										{
											RemoveObject(modelGroupIdx, entityIdx);
											for (int k = i; k < m_entites.size(); ++k)
											{
												m_entites[k]->DecreaseEntityIndex();
												if (m_entites[k]->isObject())
												{
													m_entites[k]->DecreaseObjectIndex();

												}
											}
										}

									}
									ImGui::EndPopup();
								}
								ImGui::PopID();
							}
							ImGui::TreePop();
						}
					}
					else
					{
						ImGui::PushID(i);
						Light12* pLight = m_entites[i]->GetLight();
						std::string type = m_entites[i]->GetName();
						if (ImGui::Button(type.c_str()))
						{
							UnselectModel();

							m_isObject = false;
							m_pSelectedEntity = pLight;
							ImGui::OpenPopup("Edit");
						}
						if (ImGui::BeginPopup("Edit"))
						{
							ImGui::Text("Edit");
							ImGui::Separator();
							if (ImGui::Selectable(actions))
							{
								int lightIdx = m_entites[i]->GetLightIndex();
								eLIGHT_TYPE type = pLight->GetType();
								RemoveLight(lightIdx, entityIdx, pLight);
								for (int j = i; j < m_entites.size(); ++j)
								{
									m_entites[j]->DecreaseEntityIndex();
									if (!m_entites[j]->isObject() &&
										(m_entites[j]->GetLight()->GetType()) == type)
									{
										m_entites[j]->DecreaseLightIndex();
									}
								}
							}
							ImGui::EndPopup();
						}
						ImGui::PopID();
					}
				}
				ImGui::TreePop();
			}


		}
		ImGui::End();
		if (ImGui::Begin("Properties"))
		{
			if (m_pSelectedEntity != nullptr)
			{
				if (m_isObject)
				{
					bool bDirty = false;
					Mesh* pMesh = (Mesh*)m_pSelectedEntity;
					std::string name = pMesh->GetName();
					ImGui::Text(name.c_str());

					DirectX::XMMATRIX* prevOutlinerMat = nullptr;
					DirectX::XMMATRIX* scMat = nullptr;
					DirectX::XMMATRIX* rtMat = nullptr;
					DirectX::XMMATRIX* trMat = nullptr;

					static float dragFactor = 0.1f;

					scMat = pMesh->GetScaleMatrix();
					if (scMat != nullptr)
					{
						DirectX::XMFLOAT4X4 scMat4;
						DirectX::XMStoreFloat4x4(&scMat4, *scMat);

						float prevScale[3] = { scMat4._11, scMat4._22, scMat4._33 };
						float scale[3] = { scMat4._11, scMat4._22, scMat4._33 };
						DrawVec3Control("scale", scale);

						for (int i = 0; i < 3; ++i)
						{
							if (prevScale[i] != scale[i])
							{
								bDirty = true;
								for (int i = 0; i < 3; ++i)
								{
									scale[i] = scale[i] < 0.1f ? 0.1f : scale[i];
								}

								DirectX::XMVECTOR xv = DirectX::XMVectorSet(
									scale[0],
									scale[1],
									scale[2],
									1.0f);
								DirectX::XMMATRIX sc = DirectX::XMMatrixScalingFromVector(xv);
								*scMat = sc;

								xv = DirectX::XMVectorSet(scale[0] * 1.01f, scale[1] * 1.01f, scale[2] * 1.01f, 1.0f);
								DirectX::XMMATRIX outlinerMat = DirectX::XMMatrixScalingFromVector(xv);
								prevOutlinerMat = pMesh->GetOutlinerScaleMatrix();
								if (prevOutlinerMat != nullptr)
								{
									*prevOutlinerMat = outlinerMat;
								}
								break;
							}
						}


					}

					rtMat = pMesh->GetRoatationMatrix();
					if (rtMat != nullptr)
					{
						DirectX::XMFLOAT3 angleFloat;
						DirectX::XMVECTOR* angleVec = pMesh->GetAngle();
						DirectX::XMStoreFloat3(&angleFloat, *angleVec);

						float prevAngle[3] = { angleFloat.x, angleFloat.y, angleFloat.z };
						float newAngle[3] = { angleFloat.x, angleFloat.y, angleFloat.z };
						DrawVec3Control("Rotation", newAngle);

						for (int i = 0; i < 3; ++i)
						{
							if (prevAngle[i] != newAngle[i])
							{
								bDirty = true;
								XMVECTOR newAngleVec = DirectX::XMVectorSet(newAngle[0], newAngle[1], newAngle[2], 0.0f);
								XMMATRIX rt = DirectX::XMMatrixRotationRollPitchYawFromVector(newAngleVec);
								*rtMat = rt;
								*angleVec = newAngleVec;

								break;
							}
						}


					}

					trMat = pMesh->GetTranslationMatrix();
					if (trMat != nullptr)
					{
						DirectX::XMFLOAT4X4 trMat4;
						DirectX::XMStoreFloat4x4(&trMat4, *trMat);

						float prevPos[3] = { trMat4._41, trMat4._42, trMat4._43 };
						float newPos[3] = { trMat4._41, trMat4._42, trMat4._43 };

						DrawVec3Control("Position", newPos);
						for (int i = 0; i < 3; ++i)
						{
							if (prevPos[i] != newPos[i])
							{
								bDirty = true;
								DirectX::XMVECTOR xv = DirectX::XMVectorSet(newPos[0], newPos[1], newPos[2], 0.0f);
								DirectX::XMMATRIX tr = DirectX::XMMatrixTranslationFromVector(xv);
								*trMat = tr;
								break;
							}
						}


					}

					if (bDirty)
					{
						pMesh->UpdateWorldMatrix();
						MatBuffer12* pMatBuffer = pMesh->GetMatBuffer();
						pMatBuffer->SetDirtyBit();
					}

					int texType;
					if (ImGui::Combo("Texture", &texType, " Diffuse\0 Normal\0 Specular\0 Emissive\0 Alpha\0"))
					{
						m_pTextureSrv = pMesh->GetTextureSrv(0, (eTexType)texType);
					}
					
					
					if (m_pTextureSrv != nullptr)
					{
						ImGui::Image((ImTextureID)m_pTextureSrv->ptr, ImVec2(128.f, 128.f));
					}
				}
				else
				{
					Light12* pLight = (Light12*)m_pSelectedEntity;
					DrawLightControl(pLight);
					switch (pLight->GetType())
					{
					case eLIGHT_TYPE::CUBE:
						DrawCubeLightControl(pLight);
						break;
					case eLIGHT_TYPE::SPT:
						DrawSpotLightControl(pLight);
						break;
					}
					pLight->UpdateProperty();
				}
			}

		}
		ImGui::End();

	}
	void Scene12::DrawVec3Control(const std::string& label, float* vals)
	{
		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 70.f);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		float itemWidth = ImGui::CalcItemWidth() / 3.0f;

		ImGui::PushItemWidth(itemWidth);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGuiCol_Button);
		if (ImGui::Button("X"))
		{
			vals[0] = 0.0f;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##X", &vals[0], 0.1f); //##으로 중복되는 label처리, 출력은 안됨
		ImGui::PopItemWidth();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();

		ImGui::PushItemWidth(itemWidth);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGuiCol_Button);
		if (ImGui::Button("Y"))
		{
			vals[1] = 0.0f;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##Y", &vals[1], 0.1f);
		ImGui::PopItemWidth();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();

		ImGui::PushItemWidth(itemWidth);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.35f, 0.9f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGuiCol_Button);
		if (ImGui::Button("Z"))
		{
			vals[2] = 0.0f;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##Z", &vals[2], 0.1f);
		ImGui::PopItemWidth();
		ImGui::PopStyleColor(3);

		ImGui::PopStyleVar();
		ImGui::Columns(1);

		ImGui::PopID();
	}
	void Scene12::Pick(float sx, float sy, int width, int height)
	{
		using namespace DirectX;
		XMMATRIX projMat = *(m_pCam->GetProjectionMatrix());
		XMFLOAT4X4 projMat4;
		XMStoreFloat4x4(&projMat4, projMat);

		XMMATRIX viewMat = *(m_pCam->GetViewMatrix());
		XMMATRIX invViewMat = XMMatrixInverse(nullptr, viewMat);
		XMVECTOR pos = *(m_pCam->GetPosition());
		XMFLOAT4 pos4;
		XMStoreFloat4(&pos4, pos);

		float zNDC = projMat4._33 + projMat4._43 * (1 / pos4.z);
		float vx = (2.0f * sx / width - 1.0f) / projMat4._11;
		float vy = (-2.0f * sy / height + 1.0f) / projMat4._22;
		float closestDistance = FLT_MAX;
		float hitDistance;

		m_pSelectedEntity = nullptr;
		m_pD3D12->PickSubMesh(-1, -1);
		for (int i = 0; i < m_entites.size(); ++i)
		{
			if (!m_entites[i]->isObject())
			{
				continue;
			}
			Object* pModelGroup = m_entites[i]->GetpObject();
			std::vector<Mesh*> pModels = pModelGroup->GetMeshes();
			for (int j = 0; j < pModels.size(); ++j)
			{
				Mesh* pModel = pModels[j];

				XMMATRIX m_worldMat = pModel->GetTransformMatrix(false);
				m_worldMat = DirectX::XMMatrixTranspose(m_worldMat);
				XMMATRIX invWorldMat = XMMatrixInverse(nullptr, m_worldMat);
				XMMATRIX toLocal = XMMatrixMultiply(invViewMat, invWorldMat);

				XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
				XMVECTOR v = XMVectorSet(sx, sy, zNDC, 1.0f);
				XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 1.0f);
				XMVECTOR wo = XMVector3Unproject(v, 0, 0, width, height, 0, 1, projMat, viewMat, m_worldMat);

				rayOrigin = XMVector3TransformCoord(rayOrigin, invViewMat);
				rayDir = XMVector3TransformNormal(rayDir, invViewMat);

				rayDir = XMVector3Normalize(rayDir);

				XMFLOAT3 xfO;
				XMStoreFloat3(&xfO, rayOrigin);
				XMFLOAT3 xfDir;
				XMStoreFloat3(&xfDir, rayDir);

				if (RaySphereIntersect(xfO, xfDir, 0.5f, &hitDistance) == true)
				{
					m_isObject = true;
					if (hitDistance < closestDistance)
					{
						m_pSelectedEntity = pModel;
						m_pD3D12->PickSubMesh(i, j);
						closestDistance = hitDistance;
					}
				}
			}
		}


	}
	bool Scene12::RaySphereIntersect(XMFLOAT3 o, XMFLOAT3 dir, float r, float* hitDistance)
	{
		float a = (dir.x * dir.x) + (dir.y * dir.y) + (dir.z * dir.z);
		float b = ((dir.x * o.x) + (dir.y * o.y) + (dir.z * o.z)) * 2.0f;
		float c = (o.x * o.x) + (o.y * o.y) + (o.z * o.z) - r * r;

		float discriminant = (b * b - 4.0f * a * c);
		if (discriminant < 0.0f)
		{
			return false;
		}

		*hitDistance = (-b - sqrt(discriminant)) / (2.0f * a);
		return true;
	}
	//그룹 단위로 모델이 만들어진다. 고로 하위 모델 삭제시에 RemoveEntity를 호출 해서는 안된다.
	void Scene12::RemoveSelectedModel(int modelGroupIdx, int modelIdx)
	{
		m_pD3D12->RemoveMesh(modelGroupIdx, modelIdx);
		m_pSelectedEntity = nullptr;
	}
	void Scene12::DrawLightControl(Light12* pLight)
	{
		DirectX::XMFLOAT3* pos3 = pLight->GetPos();
		DirectX::XMFLOAT3 copyPos3 = *pos3;

		DirectX::XMFLOAT3* dir3 = pLight->GetDir();
		DirectX::XMFLOAT3 copyDir3 = *dir3;

		ID3D12GraphicsCommandList* pCommandlist = m_pD3D12->GetCommandList();
		UINT lightIdx = pLight->GetLightIndex();
		static int mipLevel = -1;
		switch (pLight->GetType())
		{
		case eLIGHT_TYPE::DIR:
			ImGui::Text("Direction");
			if (ImGui::Button("X"))
			{
				dir3->x = 0.0f;
			}
			ImGui::SameLine();
			ImGui::DragFloat("##X", &(dir3->x), 0.01f, -1.0f, 1.0f);

			if (ImGui::Button("Y"))
			{
				dir3->y = 0.0f;
			}
			ImGui::SameLine();
			ImGui::DragFloat("##Y", &(dir3->y), 0.01f, -1.0f, 1.0f);

			if (ImGui::Button("Z"))
			{
				dir3->z = 0.0f;
			}
			ImGui::SameLine();
			ImGui::DragFloat("##Z", &(dir3->z), 0.01f, -1.0f, 1.0f);
			if (dir3->x != copyDir3.x || dir3->y != copyDir3.y || dir3->z != copyDir3.z)
			{
				((DirectionalLight12*)pLight)->UpdateLightSpaceMatrices();
			}

			if (ImGui::Combo("mipLevel", &mipLevel, " 0\0 1\0 2\0 3\0 4\0"))
			{
				m_pShadowSrv = m_pShadow->GetDirDebugSRV(pCommandlist, lightIdx, mipLevel);

			}

			break;
		default:
			ImGui::Text("Position");
			if (ImGui::Button("X"))
			{
				pos3->x = 0.0f;
			}
			ImGui::SameLine();
			ImGui::DragFloat("##X", &(pos3->x), 1.0f, -1000.0f, 1000.0f);

			if (ImGui::Button("Y"))
			{
				pos3->y = 0.0f;
			}
			ImGui::SameLine();
			ImGui::DragFloat("##Y", &(pos3->y), 1.0f, -1000.0f, 1000.0f);

			if (ImGui::Button("Z"))
			{
				pos3->z = 0.0f;
			}
			ImGui::SameLine();
			ImGui::DragFloat("##Z", &(pos3->z), 1.0f, -1000.0f, 1000.0f);
			if (pos3->x != copyPos3.x || pos3->y != copyPos3.y || pos3->z != copyPos3.z)
			{
				switch (pLight->GetType())
				{
				case eLIGHT_TYPE::CUBE:
					((CubeLight12*)pLight)->CreateShadowMatrices();
					break;
				case eLIGHT_TYPE::SPT:
					((SpotLight12*)pLight)->UpdateViewMat();
				}

			}

			switch (pLight->GetType())
			{
			case eLIGHT_TYPE::CUBE:
				if (ImGui::Combo("ShadowMap", &mipLevel, " right\0 left\0 up\0 down\0 front\0 back\0"))
				{
					m_pShadowSrv = m_pShadow->GetCubeDebugSRV(pCommandlist, lightIdx, mipLevel);
				}
				break;
			case eLIGHT_TYPE::SPT:
				ImGui::Text("ShadowMap");
				m_pShadowSrv = m_pShadow->GetSpotDebugSRV(pCommandlist, lightIdx);
			}

			break;
		}




		DirectX::XMFLOAT4 ambient4;
		DirectX::XMStoreFloat4(&ambient4, *(pLight->GetAmbient()));
		float ambient[4] = { ambient4.x, ambient4.y, ambient4.z, ambient4.w };
		ImGui::DragFloat4("Ambient", ambient, 0.0f, 100.0f);
		ambient4 = DirectX::XMFLOAT4(ambient[0], ambient[1], ambient[2], ambient[3]);
		*(pLight->GetAmbient()) = DirectX::XMLoadFloat4(&ambient4);

		DirectX::XMFLOAT4 diffuse4;
		DirectX::XMStoreFloat4(&diffuse4, *(pLight->GetDiffuse()));
		float diffuse[4] = { diffuse4.x, diffuse4.y, diffuse4.z, diffuse4.w };
		ImGui::DragFloat4("Diffuse", diffuse, 0.0f, 100.0f);
		diffuse4 = DirectX::XMFLOAT4(diffuse[0], diffuse[1], diffuse[2], diffuse[3]);
		*(pLight->GetDiffuse()) = DirectX::XMLoadFloat4(&diffuse4);

		DirectX::XMFLOAT4 specular4;
		DirectX::XMStoreFloat4(&specular4, *(pLight->GetSpecular()));
		float specular[4] = { specular4.x, specular4.y, specular4.z, specular4.w };
		ImGui::DragFloat4("Specular", specular, 0.0f, 100.0f);
		specular4 = DirectX::XMFLOAT4(specular[0], specular[1], specular[2], specular[3]);
		*(pLight->GetSpecular()) = DirectX::XMLoadFloat4(&specular4);


		if (m_pShadowSrv != nullptr)
		{
			ImGui::Image((ImTextureID)m_pShadowSrv->ptr, ImVec2(128.f, 128.f));
		}


	}
	void Scene12::DrawCubeLightControl(Light12* pLight)
	{
		CubeLight12* pCubeLight = (CubeLight12*)pLight;
		DirectX::XMFLOAT3 attenuation3 = *(pCubeLight->GetAttenuation());
		float attenuation[3] = { attenuation3.x, attenuation3.y, attenuation3.z };
		ImGui::DragFloat3("Attenuation", attenuation, 0.0f, 1.0f);
		*(pCubeLight->GetAttenuation()) = DirectX::XMFLOAT3(attenuation[0], attenuation[1], attenuation[2]);

		float range = *(pCubeLight->GetRange());
		if (ImGui::Button("Range"))
		{
			range = 0.0f;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##Range", &range, 0.1f);
		*(pCubeLight->GetRange()) = range;

	};
	void Scene12::DrawSpotLightControl(Light12* pLight)
	{

		SpotLight12* pSpotLight = (SpotLight12*)pLight;
		DirectX::XMFLOAT3 attenuation3 = *(pSpotLight->GetAttenuation());
		float attenuation[3] = { attenuation3.x, attenuation3.y, attenuation3.z };
		ImGui::DragFloat3("Attenuation", attenuation, 0.0f, 1.0f);
		*(pSpotLight->GetAttenuation()) = DirectX::XMFLOAT3(attenuation[0], attenuation[1], attenuation[2]);


		DirectX::XMFLOAT3 dir3 = *(pSpotLight->GetDirection());
		DirectX::XMFLOAT3 prevDir3 = dir3;
		ImGui::Text("Direction");
		if (ImGui::Button("X"))
		{
			dir3.x = 0.0f;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##DX", &dir3.x, 0.1f);

		if (ImGui::Button("Y"))
		{
			dir3.y = 0.0f;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##DY", &dir3.y, 0.1f);

		if (ImGui::Button("Z"))
		{
			dir3.z = 0.0f;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##DZ", &dir3.z, 0.1f);
		*(pSpotLight->GetDirection()) = dir3;
		if (dir3.x != prevDir3.x || dir3.y != prevDir3.y || dir3.z != prevDir3.z)
		{
			pSpotLight->UpdateViewMat();
		}

		float cutoff = *(pSpotLight->GetCutoff());
		if (ImGui::Button("cutoff"))
		{
			cutoff = 12.5f;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##CutOff", &cutoff, 0.01f, 0.01f, 45.0f);
		*(pSpotLight->GetCutoff()) = cutoff;

		float outerCutoff = *(pSpotLight->GetOuterCutoff());
		float oldOuterCutOff = outerCutoff;
		if (ImGui::Button("OuterCutoff"))
		{
			outerCutoff = 25.0f;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##OuterCutoff", &outerCutoff, 0.01f, 0.01f, 45.0f);
		*(pSpotLight->GetOuterCutoff()) = outerCutoff;
		if (outerCutoff != oldOuterCutOff)
		{
			pSpotLight->UpdateProjMat();
		}


	};
	void Scene12::RemoveObject(int modelGroupIdx, int entityIdx)
	{
		m_pD3D12->RemoveObject(modelGroupIdx);
		RemoveEntity(entityIdx);

	}
	void Scene12::RemoveEntity(int i)
	{
		std::string name = m_entites[i]->GetName();
		--m_entityCnt[name];
		delete m_entites[i];
		m_entites.erase(m_entites.begin() + i);
		UnselectModel();
	}
	void Scene12::RemoveLight(int lightIdx, int entityIdx, Light12* pLight)
	{
		m_pD3D12->RemoveLight(lightIdx, pLight);
		RemoveEntity(entityIdx);
	}
	void Scene12::UnselectModel()
	{
		m_pSelectedEntity = nullptr;
		m_pD3D12->PickSubMesh(-1, -1);
	}
}
