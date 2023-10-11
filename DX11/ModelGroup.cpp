#include "ModelGroup.h"
namespace wilson
{
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
		wchar_t* pName, eFileType eType,
		std::unordered_map<std::string, int> matHash,
		std::unordered_map<std::string, int> diffuseHash)
	{	
		std::wstring wstr(pName);
		m_Name = std::string(wstr.begin(), wstr.end());

		m_type = eType;
		m_pModels = pModels;
		m_materials = materials;
		m_texMaps = pDiffMaps;
		m_matHash = matHash;
		m_texHash = diffuseHash;

		for (int i = 0; i < m_pModels.size(); ++i)
		{
			m_pModels[i]->BindMaterial(m_matHash, m_materials, m_texHash, m_texMaps);
		}
	}
	ModelGroup::~ModelGroup()
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
}