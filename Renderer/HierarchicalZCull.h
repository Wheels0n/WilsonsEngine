#pragma once
#include <d3d12.h>
#include "typedef.h"

constexpr UINT _HI_Z_MIP_WIDTH = 512;
constexpr UINT _HI_Z_MIP_HEIGHT = 256;
namespace wilson
{
	using namespace Microsoft::WRL;
	class TextureWrapper;
	class HierarchicalZCull
	{
	public:
		void ReadBackResult(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT threadIndex);

		UINT* GetRbRawPtr(UINT threadIndex);
		
		void DrawMip(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT idx);
	
		void SetGenMipBarriers();
		void SetMipBarries(UINT mip);
		void SetBeginReadBarriers(UINT threadIndex);
		void SetEndReadBarriers(UINT threadIndex);
		void SetHWCullBarriers();

		void UploadDownSampleTexture(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT idx);
		void UploadMips(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT mipLevel);
		void UploadResolution(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT mip);

		void UploadAABB(XMVECTOR* pVertices, UINT threadIndex, UINT nMeshes);
		void UploadMatrix(XMMATRIX* pMatrices, UINT threadIndex, UINT nMeshes);
		void UploadCullTexture(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT threadIndex);
		void UploadCullData(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT threadIndex);

		HierarchicalZCull();
		~HierarchicalZCull();
	private:
		UINT m_aabbCbKey[_WORKER_THREAD_COUNT];
		UINT m_hiZCullMatrixCbKey[_WORKER_THREAD_COUNT];
		UINT m_hiZCullRbKey[_WORKER_THREAD_COUNT];
		UINT m_hiZResolutionCbKey[_HI_Z_TEX_COUNT];
		UINT m_resolutionCbKey;

		vector<D3D12_VIEWPORT>	m_hiZViewPorts;
		vector<D3D12_RECT>		m_hiZScissorRects;

		shared_ptr<TextureWrapper>		m_pHiZCullListTex[_WORKER_THREAD_COUNT];
		shared_ptr<TextureWrapper>		m_pHiZTempTex;
	};

}

