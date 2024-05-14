#include "Object12.h"
namespace wilson
{
	Object12::Object12(const std::vector<Mesh12*> pMeshes, const std::vector<MaterialInfo> materials,
		const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> texSrvs, const std::vector<ID3D12Resource*> pTexs,
		wchar_t* const pName, const eFileType eType,
		const std::unordered_map<std::string, int> matHash,
		const std::unordered_map<std::string, int> diffuseHash)
	{
		std::wstring wstr(pName);
		m_name = std::string(wstr.begin(), wstr.end());

		m_type = eType;
		m_materials = materials;
		m_texSrvs = texSrvs;
		m_matHash = matHash;
		m_texHash = diffuseHash;

		m_pTexs.resize(pTexs.size());
		for (int i = 0; i < m_pTexs.size(); ++i)
		{
			m_pTexs[i] = Microsoft::WRL::ComPtr<ID3D12Resource>(pTexs[i]);
			pTexs[i]->Release();
		}

		m_pMeshes.reserve(pMeshes.size());
		for (int i = 0; i < pMeshes.size(); ++i)
		{
			std::unique_ptr<Mesh12> uptr;
			uptr.reset(pMeshes[i]);
			m_pMeshes.push_back(std::move(uptr));
			m_pMeshes[i]->BindMaterial(m_matHash, m_materials, m_texHash, m_texSrvs);
		}
	}
	Object12::~Object12()
	{
	}
}