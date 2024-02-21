#include "ModelGroup12.h"
namespace wilson
{
	void ModelGroup12::ToggleInstancing()
	{
		for (int i = 0; i < m_pModels.size(); ++i)
		{
			m_pModels[i]->ToggleInstancing();
		}
	}
	void ModelGroup12::SetNumInstance(int n)
	{
		for (int i = 0; i < m_pModels.size(); ++i)
		{
			m_pModels[i]->SetNumInstance(n);
		}
	}
	ModelGroup12::ModelGroup12(std::vector<Model12*> pModels, std::vector<MaterialInfo> materials, 
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> texSrvs, std::vector<ID3D12Resource*> pTexs,
		wchar_t* pName, eFileType eType,
		std::unordered_map<std::string, int> matHash,
		std::unordered_map<std::string, int> diffuseHash)
	{
		std::wstring wstr(pName);
		m_Name = std::string(wstr.begin(), wstr.end());

		m_type = eType;
		m_pModels = pModels;
		m_materials = materials;
		m_texSrvs = texSrvs;
		m_pTexs = pTexs;
		m_matHash = matHash;
		m_texHash = diffuseHash;

		for (int i = 0; i < m_pModels.size(); ++i)
		{
			m_pModels[i]->BindMaterial(m_matHash, m_materials, m_texHash, m_texSrvs);
		}
	}
	ModelGroup12::~ModelGroup12()
	{
		for (int i = 0; i < m_pModels.size(); ++i)
		{
			delete m_pModels[i];
		}
		m_pModels.clear();

		
		for (int i = 0; i < m_texSrvs.size(); ++i)
		{
			m_pTexs[i]->Release();
		}

		m_texSrvs.clear();
		m_pTexs.clear();
		m_materials.clear();
		m_matHash.clear();
		m_texHash.clear();

	}
}