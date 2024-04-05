#pragma once

#include<unordered_map>

#include"Mesh.h"

namespace wilson
{	

	class Object
	{
	public:
		inline int GetNumInstance() const
		{
			return m_numOfInstances;
		}

		inline std::string GetName() const
		{
			return m_Name;
		}
		inline eFileType GetType() const
		{
			return m_type;
		}
		inline std::vector<Mesh*>& GetMeshes()
		{
			return m_pMeshes;
		}

		Object(std::vector<Mesh*> pMeshes, std::vector<MaterialInfo> materials,
			std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> pTexSrvs, std::vector<ID3D12Resource*> texs,
			wchar_t* name, eFileType type,
			std::unordered_map<std::string, int> matHash,
			std::unordered_map<std::string, int> texHash);
		~Object();
	private:
		std::string           m_Name;
		eFileType             m_type;

		std::vector<Mesh*>   m_pMeshes;
		std::vector<MaterialInfo> m_materials;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_texSrvs;
		std::vector<ID3D12Resource*> m_pTexs;
		std::unordered_map<std::string, int> m_matHash;
		std::unordered_map<std::string, int> m_texHash;

		UINT m_numOfInstances;
	};
}