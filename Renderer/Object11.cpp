#include "Object11.h"
namespace wilson
{
	void Object11::ToggleInstancing()
	{
		for (int i = 0; i < m_pMeshes.size(); ++i)
		{
			m_pMeshes[i]->ToggleInstancing();
		}
	}
	void Object11::SetNumInstance(UINT n)
	{
		for (int i = 0; i < m_pMeshes.size(); ++i)
		{
			m_pMeshes[i]->SetNumInstance(n);
		}
	}
	Object11::Object11(const std::vector<Mesh11*> pModels, const std::vector<MaterialInfo> materials, const std::vector<ID3D11ShaderResourceView*> pDiffMaps,
		wchar_t* const pName, const eFileType eType,
		const std::unordered_map<std::string, int> matHash,
		const std::unordered_map<std::string, int> diffuseHash)
	{	
		std::wstring wstr(pName);
		m_name = std::string(wstr.begin(), wstr.end());

		m_type = eType;
		m_pMeshes = pModels;
		m_materials = materials;
		m_texSrvs = pDiffMaps;
		m_matHash = matHash;
		m_texHash = diffuseHash;

		for (int i = 0; i < m_pMeshes.size(); ++i)
		{
			m_pMeshes[i]->BindMaterial(m_matHash, m_materials, m_texHash, m_texSrvs);
		}
	}
	Object11::~Object11()
	{
		for (int i = 0; i < m_pMeshes.size(); ++i)
		{
			delete m_pMeshes[i];
		}
		m_pMeshes.clear();

		for (int i = 0; i < m_texSrvs.size(); ++i)
		{
			m_texSrvs[i]->Release();
		}
		m_texSrvs.clear();
		m_materials.clear();
		m_matHash.clear();
		m_texHash.clear();

	}
}