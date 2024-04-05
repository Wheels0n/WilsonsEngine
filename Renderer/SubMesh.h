#pragma once
#include <D3D12.h>
#include <DirectXMath.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <dxgicommon.h>
#include "typedef.h"
namespace wilson
{
	class MatBuffer12;

	class SubMesh
	{
	public:
		bool operator==(const SubMesh subMesh2)
		{
			if (this->m_indexCount != subMesh2.m_indexCount ||
				this->m_matName != subMesh2.m_matName)
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		inline MatBuffer12* GetMatBuffer() const
		{
			return m_pMatBuffer;
		}
		inline PerModel* GetPerModel()
		{
			return &m_PerModel;
		}
		inline UINT GetIndexCount()
		{
			return m_indexCount;
		}
		inline std::string GetMaterialName() const
		{
			return m_matName;
		}

		static bool SortSubMeshByMaterial(const SubMesh* pSubMesh1, const SubMesh* pSubMesh2);

		SubMesh(D3D12_VERTEX_BUFFER_VIEW, D3D12_INDEX_BUFFER_VIEW, UINT indexCount,
			std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textures,
			MatBuffer12*, PerModel, std::string);
		void SetVBandIB(ID3D12GraphicsCommandList* pCommandList);
		void SetTexture(ID3D12GraphicsCommandList* pCommandList, ePass curPass);
	private:
		D3D12_VERTEX_BUFFER_VIEW m_vbV;
		D3D12_INDEX_BUFFER_VIEW m_ibV;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_pTextures;

		std::string m_matName;

		MatBuffer12* m_pMatBuffer;
		PerModel m_PerModel;

		UINT m_indexCount;
	};
}