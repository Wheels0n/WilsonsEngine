#pragma once
#include<unordered_map>
#include"Model.h"
namespace wilson
{	
	class ModelGroup
	{
	public:
		void Init(ID3D11Device* pDevice);
		void Clear();
		void ToggleInstancing();
		void SetNumInstance(int n);

		inline int GetNumInstance()
		{
			return m_numOfInstances;
		}

		inline std::string GetName()
		{
			return m_Name;
		}
		inline EFileType GetType()
		{
			return m_type;
		}
		inline std::vector<Model*>& GetModels()
		{
			return m_pModels;
		}

		ModelGroup(std::vector<Model*> pModels, std::vector<MaterialInfo> materials, std::vector<ID3D11ShaderResourceView*> pDiffMaps,
			wchar_t* name, EFileType type, 
			std::unordered_map<std::string, int> matHash,
			std::unordered_map<std::string, int> diffuseHash );
		~ModelGroup();
	 private:
		std::string           m_Name;
		EFileType             m_type;

		std::vector<Model*>   m_pModels;
		std::vector<MaterialInfo> m_materials;
		std::vector<ID3D11ShaderResourceView*> m_diffuseMaps;

		std::unordered_map<std::string, int> m_matHash, m_diffuseHash;
		int m_numOfInstances;
	};
}