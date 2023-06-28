#include "ModelGroup.h"
namespace wilson
{
	void ModelGroup::Init(ID3D11Device* pDevice)
	{
		for (int i = 0; i < m_pModels.size(); ++i)
		{	
			std::string matName = m_pModels[i]->GetMaterialName();
			int index = m_matHash[matName];
			MaterialInfo* matInfo = &m_materials[index];

			index = m_diffuseHash[matInfo->diffuseMap];
			ID3D11ShaderResourceView* pDiffuse = m_diffuseMaps[index];

			m_pModels[i]->Init(pDevice, &(matInfo->material), pDiffuse);
		}
	}

	void ModelGroup::Clear()
	{
		for (int i = 0; i < m_pModels.size(); ++i)
		{
			delete m_pModels[i];
		}
		m_pModels.clear();
	
		for (int i = 0; i < m_diffuseMaps.size(); ++i)
		{
			m_diffuseMaps[i]->Release();
		}
		m_diffuseMaps.clear();
		m_materials.clear();
		m_matHash.clear();
		m_diffuseHash.clear();

	}
	
	void ModelGroup::ToggleInstancing()
	{
		for (int i = 0; i < m_pModels.size(); ++i)
		{
			m_pModels[i]->ToggleInstancing();
		}
	}
	void ModelGroup::SetNumInstance(int n)
	{
		for (int i = 0; i < m_pModels.size(); ++i)
		{
			m_pModels[i]->SetNumInstance(n);
		}
	}
	ModelGroup::ModelGroup(std::vector<Model*> pModels, std::vector<MaterialInfo> materials, std::vector<ID3D11ShaderResourceView*> pDiffMaps,
		wchar_t* pName, EFileType type,
		std::unordered_map<std::string, int> matHash,
		std::unordered_map<std::string, int> diffuseHash)
	{	
		std::wstring wstr(pName);
		m_Name = std::string(wstr.begin(), wstr.end());

		m_type = type;
		m_pModels = pModels;
		m_materials = materials;
		m_diffuseMaps = pDiffMaps;
		m_matHash = matHash;
		m_diffuseHash = diffuseHash;
	}
	ModelGroup::~ModelGroup()
	{
		Clear();
	}
}