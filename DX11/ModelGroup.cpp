#include "ModelGroup.h"
namespace wilson
{
	void ModelGroup::Init(ID3D11Device* pDevice)
	{
		for (int i = 0; i < m_pModels.size(); ++i)
		{	
			std::string matName = m_pModels[i]->GetMaterialName();
			int index = m_matHash[matName];

			m_pModels[i]->Init(pDevice, m_materials[index], m_texHash, m_texMaps);
		}
	}

	void ModelGroup::Clear()
	{
		for (int i = 0; i < m_pModels.size(); ++i)
		{
			delete m_pModels[i];
		}
		m_pModels.clear();
	
		for (int i = 0; i < m_texMaps.size(); ++i)
		{
			m_texMaps[i]->Release();
		}
		m_texMaps.clear();
		m_materials.clear();
		m_matHash.clear();
		m_texHash.clear();

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
		m_texMaps = pDiffMaps;
		m_matHash = matHash;
		m_texHash = diffuseHash;
	}
	ModelGroup::~ModelGroup()
	{
		Clear();
	}
}