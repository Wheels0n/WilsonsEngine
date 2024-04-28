#pragma once

#include"Mesh12.h"

namespace wilson
{	

	class Object12
	{
	public:
		inline std::vector<Mesh12*>& GetMeshes()
		{
			return m_pMeshes;
		}
		inline std::string GetName() const
		{
			return m_name;
		}
		inline UINT GetNumInstance() const
		{
			return m_nInstances;
		}
		inline eFileType GetType() const
		{
			return m_type;
		}

		Object12(const std::vector<Mesh12*> pMeshes, const std::vector<MaterialInfo> materials,
			const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> pTexSrvs, const std::vector<ID3D12Resource*> texs,
			wchar_t*const name, const eFileType type,
			const std::unordered_map<std::string, int> matHash,
			const std::unordered_map<std::string, int> texHash);
		~Object12();
	private:
		std::string           m_name;
		eFileType             m_type;

		std::vector<MaterialInfo> m_materials;
		std::vector<Mesh12*>   m_pMeshes;
		std::vector<ID3D12Resource*> m_pTexs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_texSrvs;
		std::unordered_map<std::string, int> m_matHash;
		std::unordered_map<std::string, int> m_texHash;

		UINT m_nInstances;
	};
}