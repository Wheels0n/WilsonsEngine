#include "Scene.h"

CScene::CScene()
{
	m_pCD3D11 = nullptr;
	m_pSelectionETT = nullptr;
	SceneHandler = this;
	m_pCCam = nullptr;
}

CScene::~CScene()
{   
	for (int i = 0; i < m_entites.size(); ++i)
	{
		delete m_entites[i];
	}
	m_entites.clear();
	m_entites.shrink_to_fit();
}

void CScene::Init(CD3D11* pCD3D11)
{
	m_pCD3D11 = pCD3D11;
}

void CScene::AddEntity(Model* pModel)
{   
	std::wstring wStr(pModel->GetName());
	std::string name = std::string(wStr.begin(), wStr.end());
	name += std::to_string(++m_entityCnt[name]);

	CEntity* ENTT = new CEntity(name, pModel);
	m_entites.push_back(ENTT);
}

void CScene::Draw()
{   
	const char* actions = "Remove";
	if (ImGui::Begin("Scene Hierarchy"))
	{   
		if (ImGui::TreeNode(m_name.c_str()))
		{   
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			for (int i = 0; i < m_entites.size(); ++i)
			{
				std::string name = *(m_entites[i]->GetType());
                
				ImGui::PushID(i);
				if (ImGui::Button(name.c_str()))
				{   
					m_pSelectionETT = m_entites[i];
					ImGui::OpenPopup("Edit");
				}
				if (ImGui::BeginPopup("Edit"))
				{
					ImGui::Text("Edit");
					ImGui::Separator();
					if (ImGui::Selectable(actions))
					{
						RemoveENTT(i);
					}
					ImGui::EndPopup();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}

		ImGui::End();
	}

	if (ImGui::Begin("Properties"))
	{   
		if (m_pSelectionETT != nullptr)
		{   
			std::string name = *(m_pSelectionETT->GetType());
			ImGui::Text(name.c_str());
			Model* pModel = m_pSelectionETT->GetModel();

			DirectX::XMMATRIX* scMat = nullptr;
			DirectX::XMMATRIX* rtMat = nullptr;
			DirectX::XMMATRIX* trMat = nullptr;

			static float dragFactor = 0.1f;

			scMat = pModel->GetScaleMatrix();
			if (scMat != nullptr)
			{
				DirectX::XMFLOAT4X4 scMat4;
				DirectX::XMStoreFloat4x4(&scMat4, *scMat);

				float scale[3] = { scMat4._11, scMat4._22, scMat4._33 };
				DrawVec3Control("scale", scale);
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
				
			}
			
			rtMat = pModel->GetRoatationMatrix();
			if (rtMat != nullptr)
			{
				DirectX::XMFLOAT3 angleFloat;
				DirectX::XMVECTOR* angleVec = pModel->GetAngle();
				DirectX::XMStoreFloat3(&angleFloat, *angleVec);

				float newAngle[3] = { angleFloat.x, angleFloat.y, angleFloat.z };
				DrawVec3Control("Rotation", newAngle);
				XMVECTOR newAngleVec = DirectX::XMVectorSet(newAngle[0], newAngle[1], newAngle[2], 0.0f );
				XMMATRIX rt = DirectX::XMMatrixRotationRollPitchYawFromVector(newAngleVec);
				*rtMat = rt;
				*angleVec = newAngleVec;
  

			}

			trMat = pModel->GetTranslationMatrix();
			if (trMat != nullptr)
			{   
				DirectX::XMFLOAT4X4 trMat4;
				DirectX::XMStoreFloat4x4(&trMat4, *trMat);

				float newPos[3] = { trMat4._41, trMat4._42, trMat4._43 };
				DrawVec3Control("Position", newPos);
				
				DirectX::XMVECTOR xv = DirectX::XMVectorSet(newPos[0], newPos[1], newPos[2], 0.0f);
				DirectX::XMMATRIX tr = DirectX::XMMatrixTranslationFromVector(xv);
				*trMat = tr;

			}
		
		}
		ImGui::End();
	}
}

void CScene::DrawVec3Control(const std::string& label, float* vals)
{  
	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, 70.f);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	float itemWidth =ImGui::CalcItemWidth() / 3.0f;

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

void CScene::Pick(int sx, int sy, int width, int height)
{   
	//m_pSelectionETT = nullptr;

	using namespace DirectX;

	XMMATRIX projectionMat = *(m_pCCam->GetProjectionMatrix());
	XMFLOAT4X4 projectionMat4;
	XMStoreFloat4x4(&projectionMat4, projectionMat);

	XMMATRIX viewMat = *(m_pCCam->GetViewMatrix());
	XMMATRIX inverseView = XMMatrixInverse(nullptr, viewMat);
	XMVECTOR pos = *(m_pCCam->GetPosition());
	XMFLOAT4 pos4;
	XMStoreFloat4(&pos4, pos);

	float zNDC = projectionMat4._33 + projectionMat4._43* (1/pos4.z);
	float vx = (2.0f * sx / width - 1.0f) / projectionMat4._11;
	float vy = (-2.0f * sy / height + 1.0f) / projectionMat4._22;
	float closestDistance = FLT_MAX;
	float hitDistance;

	for (int i = 0; i < m_entites.size(); ++i)
	{   
		Model* pModel = m_entites[i]->GetModel();

		XMMATRIX m_worldMat = pModel->GetTransformMatrix();
		XMMATRIX inverseWorld = XMMatrixInverse(nullptr, m_worldMat);
		XMMATRIX toLocal = XMMatrixMultiply(inverseView, inverseWorld);

		XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
		XMVECTOR v = XMVectorSet(sx, sy, zNDC, 1.0f);
		XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 1.0f);
		XMVECTOR wo =XMVector3Unproject(v, 0, 0, width, height, 0, 1, projectionMat, viewMat, m_worldMat);

		rayOrigin = XMVector3TransformCoord(rayOrigin, inverseView);
		rayDir = XMVector3TransformNormal(rayDir, inverseView);

		rayDir = XMVector3Normalize(rayDir);
		
		XMFLOAT3 xfO;
		XMStoreFloat3(&xfO, rayOrigin);
		XMFLOAT3 xfDir;
		XMStoreFloat3(&xfDir, rayDir);

		if (RaySphereIntersect(xfO, xfDir, 0.5f, &hitDistance) == true)
		{   
			if (hitDistance < closestDistance)
			{
				m_pSelectionETT = m_entites[i];
				closestDistance = hitDistance;
			}
		}
		
	}

	
}

bool CScene::RaySphereIntersect(XMFLOAT3 o, XMFLOAT3 dir, float r, float* hitDistance)
{
	float a = (dir.x * dir.x) + (dir.y * dir.y) + (dir.z * dir.z);
	float b = ((dir.x * o.x) + (dir.y * o.y) + (dir.z * o.z)) * 2.0f;
	float c = (o.x * o.x) + (o.y * o.y) + (o.z * o.z) - r*r;
	
	float discriminant = (b * b - 4.0f * a * c);
	if (discriminant < 0.0f)
	{
		return false;
	}

	*hitDistance = (-b - sqrt(discriminant)) / (2.0f * a);
	return true;
}

void CScene::RemoveENTT(int i)
{  
	m_pCD3D11->RemoveModel(i);

	std::string type = *(m_entites[i]->GetType());
	--m_entityCnt[type];
	delete m_entites[i];
	m_entites.erase(m_entites.begin()+i);

	m_pSelectionETT = nullptr;
}

