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
				float vec[3] = { mat4._41, mat4._42, mat4._43 };

				ImGui::DragFloat3("Translation", vec);
			}
	
		}
		ImGui::End();
	}
}

