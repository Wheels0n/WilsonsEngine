#pragma once

#include<unordered_map>

#include"Model12.h"

namespace wilson
{	

	class ModelGroup12
	{
	public:
		void ToggleInstancing();
		void SetNumInstance(int n);

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
		inline std::vector<Model12*>& GetModels()
		{
			return m_pModels;
		}

		ModelGroup12(std::vector<Model12*> pModels, std::vector<MaterialInfo> materials,
			std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> pTexSrvs, std::vector<ID3D12Resource*> texs,
			wchar_t* name, eFileType type,
			std::unordered_map<std::string, int> matHash,
			std::unordered_map<std::string, int> texHash);
		~ModelGroup12();
	private:
		std::string           m_Name;
		eFileType             m_type;

		std::vector<Model12*>   m_pModels;
		std::vector<MaterialInfo> m_materials;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_texSrvs;
		std::vector<ID3D12Resource*> m_pTexs;
		std::unordered_map<std::string, int> m_matHash;
		std::unordered_map<std::string, int> m_texHash;

		UINT m_numOfInstances;
	};
}