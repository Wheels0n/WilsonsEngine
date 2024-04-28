#include "Object12.h"
namespace wilson
{
	Object12::Object12(const std::vector<Mesh12*> pModels, const std::vector<MaterialInfo> materials,
		const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> texSrvs, const std::vector<ID3D12Resource*> pTexs,
		wchar_t* const pName, const eFileType eType,
		const std::unordered_map<std::string, int> matHash,
		const std::unordered_map<std::string, int> diffuseHash)
	{
		std::wstring wstr(pName);
		m_name = std::string(wstr.begin(), wstr.end());

		m_type = eType;
		m_pMeshes = pModels;
		m_materials = materials;
		m_texSrvs = texSrvs;
		m_pTexs = pTexs;
		m_matHash = matHash;
		m_texHash = diffuseHash;

		for (int i = 0; i < m_pMeshes.size(); ++i)
		{
			m_pMeshes[i]->BindMaterial(m_matHash, m_materials, m_texHash, m_texSrvs);
		}
	}
	Object12::~Object12()
	{
		for (int i = 0; i < m_pMeshes.size(); ++i)
		{
			delete m_pMeshes[i];
		}
		m_pMeshes.clear();

		
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