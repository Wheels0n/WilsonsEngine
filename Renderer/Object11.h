#pragma once

#include"Mesh11.h"

namespace wilson
{	
	class Object11
	{

	public:
		inline std::vector<Mesh11*>& GetMeshes()
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
		void SetNumInstance(UINT n);
		void ToggleInstancing();

		Object11(const std::vector<Mesh11*> pMeshes, const std::vector<MaterialInfo> materials, 
			const std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> pTextures,
			wchar_t* const name, const eFileType type,
			const std::unordered_map<std::string, int> matHash,
			const std::unordered_map<std::string, int> texHash );
		~Object11();
	 private:
		std::string           m_name;
		eFileType             m_type;

		std::vector<MaterialInfo> m_materials;
		std::vector<Mesh11*>   m_pMeshes;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_texSrvs;
		std::unordered_map<std::string, int> m_matHash;
		std::unordered_map<std::string, int> m_texHash;

		UINT m_nInstances;
	};
}