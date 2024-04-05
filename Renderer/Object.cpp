#include "Object.h"
namespace wilson
{
	Object::Object(std::vector<Mesh*> pModels, std::vector<MaterialInfo> materials, 
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> texSrvs, std::vector<ID3D12Resource*> pTexs,
		wchar_t* pName, eFileType eType,
		std::unordered_map<std::string, int> matHash,
		std::unordered_map<std::string, int> diffuseHash)
	{
		std::wstring wstr(pName);
		m_Name = std::string(wstr.begin(), wstr.end());

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
	Object::~Object()
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