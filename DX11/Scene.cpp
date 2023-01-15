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

void CScene::AddEntity(std::string Name, DirectX::XMMATRIX* worldMat)
{   
	CEntity* ENTT = new CEntity(Name, worldMat);
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
 
			DirectX::XMMATRIX* mat = nullptr;
			mat = m_pSelectionETT->GetMat();
			if (mat != nullptr)
			{   
				DirectX::XMFLOAT4X4 mat4;
				DirectX::XMStoreFloat4x4(&mat4, *mat);
				float curPos[3] = { mat4._41, mat4._42, mat4._43 };
				float newPos[3] = { mat4._41, mat4._42, mat4._43 };

				static float dragFactor = 0.1f;
				if (ImGui::DragFloat3("Translation", newPos, dragFactor))
				{
					float v[3] = { 0.0f, };
					if (newPos[0] != curPos[0])
					{   
						float val = newPos[0] - curPos[0];
						v[0] = val;
					}

					if (newPos[1] != curPos[1])
					{
						float val = newPos[1] - curPos[1];
						v[1] = val;
					}

					if (newPos[2] != curPos[2])
					{
						float val = newPos[2] - curPos[2];
						v[2] = val;
					}

					DirectX::XMVECTOR xv = DirectX::XMVectorSet(v[0], v[1], v[2], 0.0f);
					DirectX::XMMATRIX tr = DirectX::XMMatrixTranslationFromVector(xv);
					*mat = DirectX::XMMatrixMultiply(*mat, tr);
				}

			}
	
		}
		ImGui::End();
	}
}

