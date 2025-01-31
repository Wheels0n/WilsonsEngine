#include "Object12.h"
namespace wilson
{
	MeshGroup12::MeshGroup12(const vector<shared_ptr<Mesh12>>& pMeshes, const vector<MaterialInfo>& materials,
		 const vector<shared_ptr<TextureWrapper>>& pTexs,
		LPCWSTR pName, const eFileType eType,
		const unordered_map<string, int>& matHash,
		const unordered_map<string, int>& diffuseHash)
		: m_type(eType), m_materials(materials),  m_matHash(matHash), m_texHash(diffuseHash)
	{
		wstring wstr(pName);
		m_name = string(wstr.begin(), wstr.end());

		m_pTexs =pTexs;
		m_pMeshes = pMeshes;

		for (int i = 0; i < m_pMeshes.size(); ++i)
		{
			m_pMeshes[i]->PostInitMeshlets();
			m_pMeshes[i]->BindMaterial(m_matHash, materials, m_texHash, m_pTexs);
		}
	}

	vector<shared_ptr<Mesh12>>& MeshGroup12::GetMeshes()
	{
		return m_pMeshes;
	}
	string&						MeshGroup12::GetName() 
	{
		return m_name;
	}
	INT							MeshGroup12::GetNumInstance() 
	{
		return m_nInstances;
	}
	eFileType					MeshGroup12::GetType() 
	{
		return m_type;
	}

}