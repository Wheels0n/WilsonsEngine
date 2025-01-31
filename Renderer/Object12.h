#pragma once

#include"Mesh12.h"

namespace wilson
{	
	using namespace std;
	using namespace DirectX;
	using namespace Microsoft::WRL;

	class MeshGroup12 : enable_shared_from_this<MeshGroup12>
	{
		public:
			
			INT							GetNumInstance();
			string&						GetName();
			eFileType					GetType();
			vector<shared_ptr<Mesh12>>& GetMeshes();

			MeshGroup12(const vector<shared_ptr<Mesh12>>& pMeshes, const vector<MaterialInfo>& materials,
				const vector<shared_ptr<TextureWrapper>>& texs,
				LPCWSTR pName, const eFileType type,
				const unordered_map<string, int>& matHash,
				const unordered_map<string, int>& texHash);
			~MeshGroup12() = default;
		private:
			string						m_name;
			eFileType					m_type;

			vector<MaterialInfo>		m_materials;
			vector<shared_ptr<Mesh12>>  m_pMeshes;
			vector<shared_ptr<TextureWrapper>>	m_pTexs;
			unordered_map<string, int> m_matHash;
			unordered_map<string, int> m_texHash;

			UINT m_nInstances;
	};
}