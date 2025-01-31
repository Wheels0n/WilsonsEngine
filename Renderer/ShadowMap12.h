#pragma once
#include <d3d12.h>
#include "typedef.h"
#include "TextureManager.h"
namespace wilson
{
	using namespace std;
	using namespace DirectX;
	using namespace Microsoft::WRL;
	
	class HeapManager;
	class ShadowMap12
	{
	private:
		void								SetShadowViewport(ComPtr<ID3D12GraphicsCommandList>);
		void								SetShadowRect(ComPtr<ID3D12GraphicsCommandList>);
		void								SetResourceBarrier(ComPtr<ID3D12GraphicsCommandList>, const UINT litCnts[],
			const D3D12_RESOURCE_STATES before, const D3D12_RESOURCE_STATES after, const bool bRTV);
	public:
		void								ClearDsv(ComPtr<ID3D12GraphicsCommandList> pCmdList, const UINT litCounts[]);
		void								ClearRtv(ComPtr<ID3D12GraphicsCommandList> pCmdList, const UINT litCounts[]);
		
		D3D12_GPU_DESCRIPTOR_HANDLE*		GetCubeDebugSrv(ComPtr<ID3D12GraphicsCommandList> pCmdList, const UINT i, const UINT face);
		D3D12_GPU_DESCRIPTOR_HANDLE*		GetDirDebugSrv(ComPtr<ID3D12GraphicsCommandList> pCmdList, const UINT i, const UINT lod);
		D3D12_GPU_DESCRIPTOR_HANDLE*		GetSpotDebugSrv(ComPtr<ID3D12GraphicsCommandList> pCmdList, const UINT i);

		void								SetClearViewBarrier(ComPtr<ID3D12GraphicsCommandList>, const UINT litCnts[]);
		void								SetLightingPassBarrier(ComPtr<ID3D12GraphicsCommandList>, const UINT litCnts[]);

		void								SetDirPass(ComPtr<ID3D12GraphicsCommandList>, UINT lightIdx);
		void								SetCubePass(ComPtr<ID3D12GraphicsCommandList>, UINT lightIdx);
		void								SetSpotPass(ComPtr<ID3D12GraphicsCommandList>, UINT lightIdx);
		void								SetLightingPass(ComPtr<ID3D12GraphicsCommandList>);
	

											ShadowMap12(ComPtr<ID3D12Device> pDevice,
												const UINT width, const UINT height, const UINT cascadeLevel,
												const UINT dirLightCap, const UINT pntLightCap, const UINT spotLightCap);
											~ShadowMap12() =default;

	private:
		UINT								m_width;
		UINT								m_height;
		FLOAT								m_clear[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

		//D3D12
		vector<shared_ptr<TextureWrapper>>		m_cubeTexes;
		vector<shared_ptr<TextureWrapper>>		m_cubeDebugTexes;
		vector<shared_ptr<TextureWrapper>>		m_debugTexes;
		vector<shared_ptr<TextureWrapper>>		m_dirTexes;
		vector<shared_ptr<TextureWrapper>>		m_dirDebugTexes;
		vector<shared_ptr<TextureWrapper>>		m_spotTexes;
		vector<shared_ptr<TextureWrapper>>		m_spotDebugTexes;

		D3D12_VIEWPORT						m_viewport;
		D3D12_RECT							m_rect = { 0, };
	};
};