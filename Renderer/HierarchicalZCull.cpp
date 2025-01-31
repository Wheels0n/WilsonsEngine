#include "HierarchicalZCull.h"
#include "HeapManager.h"
#include "TextureManager.h"
#include "ResourceBarrierHandler.h"
namespace wilson
{
	void HierarchicalZCull::ReadBackResult(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT threadIndex)
	{
		//ReadBackResult;
		{
			UINT len = sqrt(_HI_Z_CULL_COUNT);
			D3D12_TEXTURE_COPY_LOCATION dst = {};
			dst.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R32_UINT;
			dst.PlacedFootprint.Footprint.Width = len;
			dst.PlacedFootprint.Footprint.RowPitch = CUSTUM_ALIGN(sizeof(UINT) * len, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			dst.PlacedFootprint.Footprint.Height = len;
			dst.PlacedFootprint.Footprint.Depth = 1;

			dst.pResource = g_pHeapManager->GetRbBlock(m_hiZCullRbKey[threadIndex]).Get();
			dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

			D3D12_TEXTURE_COPY_LOCATION src = {};
			src.pResource = m_pHiZCullListTex[threadIndex]->tex.Get();
			src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			src.SubresourceIndex = 0;

			pCmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		}
	}
	UINT* HierarchicalZCull::GetRbRawPtr(UINT threadIndex)
	{
		return reinterpret_cast<UINT*>(g_pHeapManager->GetRBRawPtr(m_hiZCullRbKey[threadIndex]));
	}
	void HierarchicalZCull::DrawMip(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT idx)
	{
		pCmdList->Dispatch(m_hiZViewPorts[idx].Width, m_hiZViewPorts[idx].Height, 1);
	}

	void HierarchicalZCull::SetGenMipBarriers()
	{
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrierWithMip(m_pHiZTempTex,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 0));
	}
	void HierarchicalZCull::SetMipBarries(UINT mip)
	{
		UINT mipLevels = 1 + (UINT)floorf(log2f(fmaxf(_SHADOWMAP_SIZE, _SHADOWMAP_SIZE / 2)));

		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrierWithMip(m_pHiZTempTex,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, mip));
	}
	void HierarchicalZCull::SetBeginReadBarriers(UINT threadIndex)
	{
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pHiZCullListTex[threadIndex], 
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
	}
	void HierarchicalZCull::SetEndReadBarriers(UINT threadIndex)
	{
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pHiZCullListTex[threadIndex],
				D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	}
	void HierarchicalZCull::SetHWCullBarriers()
	{
		UINT mipLevels = 1 + (UINT)floorf(log2f(fmaxf(_SHADOWMAP_SIZE, _SHADOWMAP_SIZE / 2)));

		for (int i = 0; i < mipLevels; ++i)
		{
			g_pResourceBarrierHandler->Push(
				g_pTextureManager->CreateBarrierWithMip(
					m_pHiZTempTex, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS, i));
		}
	}
	void HierarchicalZCull::UploadDownSampleTexture(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT idx)
	{
		g_pTextureManager->UploadComputeUavMip(pCmdList,m_pHiZTempTex, idx, 0);
	}
	void HierarchicalZCull::UploadMips(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT mipLevel)
	{
		g_pTextureManager->UploadComputeSrvMip(pCmdList, m_pHiZTempTex, E_TO_UINT(eGenHiZRP::psLastMip), mipLevel-1);
		g_pTextureManager->UploadComputeUavMip(pCmdList, m_pHiZTempTex, E_TO_UINT(eGenHiZRP::dst), mipLevel);
	}
	void HierarchicalZCull::UploadResolution(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT mip)
	{
		g_pHeapManager->UploadComputeCb(m_hiZResolutionCbKey[mip], E_TO_UINT(eGenHiZRP::lastResoltion), pCmdList);
	}
	void HierarchicalZCull::UploadAABB(XMVECTOR* pVertices, UINT threadIndex, UINT nMeshes)
	{
		g_pHeapManager->CopyDataToCb(m_aabbCbKey[threadIndex], sizeof(XMVECTOR) * _CUBE_VERTICES * nMeshes, pVertices);
	}
	void HierarchicalZCull::UploadMatrix(XMMATRIX* pMatrices, UINT threadIndex, UINT nMeshes)
	{
		g_pHeapManager->CopyDataToCb(m_hiZCullMatrixCbKey[threadIndex], sizeof(XMMATRIX) * nMeshes, pMatrices);
	}
	void HierarchicalZCull::UploadCullTexture(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT threadIndex)
	{
		g_pTextureManager->UploadComputeSrv(pCmdList, m_pHiZTempTex, E_TO_UINT(eHiZCullRP::csHiZ));
		g_pTextureManager->UploadComputeUav(pCmdList, m_pHiZCullListTex[threadIndex], E_TO_UINT(eHiZCullRP::csDst));
	}
	void HierarchicalZCull::UploadCullData(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT threadIndex)
	{
		g_pHeapManager->UploadComputeCb(m_resolutionCbKey, E_TO_UINT(eHiZCullRP::csResolution), pCmdList);
		g_pHeapManager->UploadComputeCb(m_aabbCbKey[threadIndex], E_TO_UINT(eHiZCullRP::csBound), pCmdList);
		g_pHeapManager->UploadComputeCb(m_hiZCullMatrixCbKey[threadIndex], E_TO_UINT(eHiZCullRP::csMatrix), pCmdList);
	}
	HierarchicalZCull::HierarchicalZCull()
	{
		UINT curWidth =	_HI_Z_MIP_WIDTH;
		UINT curHeight = _HI_Z_MIP_HEIGHT;
		UINT mipLevels = 1 + (UINT)floorf(log2f(fmaxf(_HI_Z_MIP_WIDTH, _HI_Z_MIP_HEIGHT/ 2)));
		m_hiZViewPorts.resize(mipLevels + 1);
		m_hiZScissorRects.resize(mipLevels + 1);
		for (int i = 0; i < mipLevels; ++i)
		{
			D3D12_VIEWPORT viewPort = {};
			viewPort.Width = curWidth;
			viewPort.Height = curHeight;

			D3D12_RECT scissorRect = {};
			scissorRect.right = curWidth;
			scissorRect.bottom = curHeight;

			m_hiZViewPorts[i] = viewPort;
			m_hiZScissorRects[i] = scissorRect;

			curWidth /= 2;
			curHeight /= 2;
			curWidth = max(1, curWidth);
			curHeight = max(1, curHeight);

		}

		{

			UINT mipLevels = 1 + E_TO_UINT(floorf(log2f(fmaxf(_SHADOWMAP_SIZE, _SHADOWMAP_SIZE / 2))));
			m_pHiZTempTex = g_pTextureManager->CreateTexture2D(_SHADOWMAP_SIZE, _SHADOWMAP_SIZE / 2, 1, mipLevels,
				(BYTE)0b10011, DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			SET_PRIVATE_DATA(m_pHiZTempTex->tex);

			UINT width = sqrt(_HI_Z_CULL_COUNT);
			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{

				m_pHiZCullListTex[i] = g_pTextureManager->CreateTexture2D(width, width, 1, 1,
					(BYTE)0b0001, DXGI_FORMAT_R32_UINT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				SET_PRIVATE_DATA(m_pHiZCullListTex[i]->tex);

			}

		}

		m_resolutionCbKey = g_pHeapManager->AllocateCb(sizeof(UINT) * 2);
		UINT resolution[] = { _HI_Z_MIP_WIDTH,   _HI_Z_MIP_HEIGHT };
		g_pHeapManager->CopyDataToCb(m_resolutionCbKey, sizeof(UINT) * 2, resolution);

		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			m_aabbCbKey[i] = g_pHeapManager->AllocateCb((sizeof(XMVECTOR)) * _HI_Z_CULL_COUNT * _CUBE_VERTICES);
			m_hiZCullMatrixCbKey[i] = g_pHeapManager->AllocateCb(sizeof(XMMATRIX) * _HI_Z_CULL_COUNT);
		}

		UINT lastWidth = _SHADOWMAP_SIZE;
		UINT lastHeight = _SHADOWMAP_SIZE / 2;
		for (int i = 0; i < _HI_Z_TEX_COUNT; ++i)
		{
			UINT cbSize = sizeof(UINT) * 4;
			m_hiZResolutionCbKey[i] = g_pHeapManager->AllocateCb(cbSize);

			UINT lastMipInfo[4] = { lastWidth,lastHeight,i,0 };
			g_pHeapManager->CopyDataToCb(m_hiZResolutionCbKey[i], cbSize, lastMipInfo);

			lastWidth /= 2;
			lastHeight /= 2;
			lastWidth = max(1, lastWidth);
			lastHeight = max(1, lastHeight);
		}

		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			m_hiZCullRbKey[i] = g_pHeapManager->AllocateRb(CUSTUM_ALIGN(_HI_Z_CULL_COUNT * sizeof(UINT), _64KB));
		}

	}
	HierarchicalZCull::~HierarchicalZCull()
	{
		if (m_pHiZTempTex != nullptr)
		{
			m_pHiZTempTex = nullptr;
		}

		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			if (m_pHiZCullListTex[i] != nullptr)
			{
				m_pHiZCullListTex[i] = nullptr;
			}
		}
	}
}