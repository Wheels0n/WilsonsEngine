#pragma once

#include<unordered_map>

#include"Model.h"

namespace wilson
{	
	class ModelGroup
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
		inline std::vector<Model*>& GetModels()
		{
			return m_pModels;
		}

		ModelGroup(std::vector<Model*> pModels, std::vector<MaterialInfo> materials, std::vector<ID3D11ShaderResourceView*> pTextures,
			wchar_t* name, eFileType type, 
			std::unordered_map<std::string, int> matHash,
			std::unordered_map<std::string, int> texHash );
		~ModelGroup();
	 private:
		std::string           m_Name;
		eFileType             m_type;

		std::vector<Model*>   m_pModels;
		std::vector<MaterialInfo> m_materials;
		std::vector<ID3D11ShaderResourceView*> m_texMaps;
		std::unordered_map<std::string, int> m_matHash;
		std::unordered_map<std::string, int> m_texHash;

		UINT m_numOfInstances;
	};
}