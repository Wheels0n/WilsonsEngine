#include "Scene.h"

CScene::~CScene()
{   
	for (int i = 0; i < m_entites.size(); ++i)
	{
		delete m_entites[i];
	}
	m_entites.clear();
	m_entites.shrink_to_fit();
}

void CScene::AddEntity(CModel* pModel)
{   
	std::wstring wStr(pModel->GetName());
	std::string name = std::string(wStr.begin(), wStr.end());
	name += std::to_string(++m_entityCnt[name]);

	CEntity* ENTT = new CEntity(name, pModel);
	m_entites.push_back(ENTT);
}

void CScene::Draw()
{
	if (ImGui::Begin("Scene Hierarchy"))
	{   
		if (ImGui::TreeNode(m_name.c_str()))
		{   
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			for (int i = 0; i < m_entites.size(); ++i)
			{
				std::string name = *(m_entites[i]->GetType());

				if (ImGui::TreeNode((void*)(intptr_t)i, name.c_str(), i))
				{
					m_pSelectionETT = m_entites[i];
					ImGui::TreePop();
				}
				
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
			CModel* pModel = m_pSelectionETT->GetModel();

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
				if (ImGui::DragFloat3("Scale", scale, dragFactor))
				{  
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
			}
			
			rtMat = pModel->GetRoatationMatrix();
			if (rtMat != nullptr)
			{
				DirectX::XMFLOAT3 angleFloat;
				DirectX::XMVECTOR* angleVec = pModel->GetAngle();
				DirectX::XMStoreFloat3(&angleFloat, *angleVec);

				float newAngle[3] = { angleFloat.x, angleFloat.y, angleFloat.z };
				if (ImGui::DragFloat3("Rotation", newAngle, dragFactor))
				{    
					XMVECTOR newAngleVec = DirectX::XMVectorSet(newAngle[0], newAngle[1], newAngle[2], 0.0f );
					XMMATRIX rt = DirectX::XMMatrixRotationRollPitchYawFromVector(newAngleVec);
					*rtMat = rt;
					*angleVec = newAngleVec;
  
				}

			}

			trMat = pModel->GetTranslationMatrix();
			if (trMat != nullptr)
			{   
				DirectX::XMFLOAT4X4 trMat4;
				DirectX::XMStoreFloat4x4(&trMat4, *trMat);

				float curPos[3] = { trMat4._41, trMat4._42, trMat4._43 };
				float newPos[3] = { trMat4._41, trMat4._42, trMat4._43 };

				if (ImGui::DragFloat3("Position", newPos, dragFactor))
				{   
					DirectX::XMVECTOR xv = DirectX::XMVectorSet(newPos[0], newPos[1], newPos[2], 0.0f);
					DirectX::XMMATRIX tr = DirectX::XMMatrixTranslationFromVector(xv);
					*trMat = tr;
				}

			}

		
		}
		ImGui::End();
	}
}

void CScene::Pick(int sx, int sy, int width, int height)
{
	using namespace DirectX;

	XMMATRIX projectionMat = *(m_pCCam->GetProjectionMatrix());
	XMFLOAT4X4 projectionMat4;
	XMStoreFloat4x4(&projectionMat4, projectionMat);

	XMMATRIX viewMat = *(m_pCCam->GetViewMatrix());
	XMVECTOR vDet = XMMatrixDeterminant(viewMat);
	XMMATRIX inverseView = XMMatrixInverse(&vDet, viewMat);

	float vx = (2.0f * (float)sx / (float)width - 1.0f) / projectionMat4._11;
	float vy = (-2.0f * (float)sy / (float)height + 1.0f) / projectionMat4._22;

	XMVECTOR rayOrigin = *(m_pCCam->GetPosition());
	XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

	for (int i = 0; i < m_entites.size(); ++i)
	{   
		CModel* pModel = m_entites[i]->GetModel();
		XMMATRIX world = pModel->GetTransformMatrix();
		XMVECTOR wDet = XMMatrixDeterminant(world);
		XMMATRIX inverseWorld = XMMatrixInverse(&wDet, world);

		XMMATRIX toLocal = XMMatrixMultiply(inverseView, inverseWorld);

		rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
		rayDir = XMVector3TransformNormal(rayDir, toLocal);

		rayDir = XMVector3Normalize(rayDir);

		XMFLOAT3 xfO;
		XMStoreFloat3(&xfO, rayOrigin);
		XMFLOAT3 xfDir;
		XMStoreFloat3(&xfDir, rayDir);

		if (RaySphereIntersect(xfO, xfDir, 1.0f) == true)
		{
			m_pSelectionETT = m_entites[i];
			break;
		}
		
	}
}

bool CScene::RaySphereIntersect(XMFLOAT3 o, XMFLOAT3 dir, float r)
{
	float a = (dir.x * dir.x) + (dir.y * dir.y) + (dir.z * dir.z);
	float b = ((dir.x * o.x) + (dir.y * o.y) + (dir.z * o.z)) * 2.0f;
	float c = (o.x * o.x) + (o.y * o.y) + (o.z * o.z) - r*r;
	
	float discriminant = (b * b - (4 * a * c));
	return discriminant<0 ? false:true;
}
