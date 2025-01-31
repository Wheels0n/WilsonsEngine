#pragma once

#pragma comment(lib, "DirectXTex.lib")
#include <wrl.h>
#include <memory>
#include <d3d12.h>
#include <dxgi.h>
#include <vector>
namespace wilson
{
	using namespace std;
	using namespace Microsoft::WRL;

	struct TextureWrapper
	{
		ComPtr<ID3D12Resource>		tex;
		ComPtr<ID3D12Resource>		cb;

		D3D12_CPU_DESCRIPTOR_HANDLE rtv;
		D3D12_CPU_DESCRIPTOR_HANDLE dsv;
		D3D12_GPU_DESCRIPTOR_HANDLE srv;
		D3D12_GPU_DESCRIPTOR_HANDLE uav;

		vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
		vector<D3D12_GPU_DESCRIPTOR_HANDLE> srvs;
		vector<D3D12_GPU_DESCRIPTOR_HANDLE> uavs;

		BOOL bCube;
		BOOL bViewPerMip;
		BOOL hasRtv;
		BOOL hasDSV;
		BOOL hasSRV;
		BOOL hasUAV;
	};

	class TextureManager :public enable_shared_from_this<TextureManager>
	{
	public:
		//View Bit : Cube/ViewPerMip/RTV/DSV/SRV/UAV

		D3D12_RESOURCE_BARRIER					CreateBarrier(shared_ptr<TextureWrapper> pTex, D3D12_RESOURCE_STATES srv, D3D12_RESOURCE_STATES dst);
		D3D12_RESOURCE_BARRIER					CreateBarrierWithMip(shared_ptr<TextureWrapper> pTex, D3D12_RESOURCE_STATES src, D3D12_RESOURCE_STATES dst, UINT mipLevel);
		
		shared_ptr<TextureWrapper>				CreateTexture2D(const UINT width, const UINT height, const UINT arraySize, const UINT mipLevels,
													BYTE view, DXGI_FORMAT format, D3D12_RESOURCE_STATES state);
		shared_ptr<TextureWrapper>				CreateTexture2D(ComPtr<IDXGISwapChain>, UINT i);

		shared_ptr<TextureWrapper>				LoadImageWICFile(const WCHAR* pPath, ComPtr<ID3D12GraphicsCommandList> pCmdList);
		shared_ptr<TextureWrapper>				LoadImageDDSFile(const WCHAR* pPath, ComPtr<ID3D12GraphicsCommandList> pCmdList);
		void									UploadTexThroughCB(const D3D12_RESOURCE_DESC texDesc, const UINT rowPitch,
														const UINT8* pData, ComPtr<ID3D12Resource>& pDst, ComPtr<ID3D12Resource>& ppUploadCB, ComPtr<ID3D12GraphicsCommandList> pCmdList);
		void									UploadGraphicsSrv(ComPtr<ID3D12GraphicsCommandList> pCmdList, shared_ptr<TextureWrapper> tex, UINT idx);
		void									UploadComputeSrv(ComPtr<ID3D12GraphicsCommandList> pCmdList, shared_ptr<TextureWrapper> tex, UINT idx);
		void									UploadComputeSrvMip(ComPtr<ID3D12GraphicsCommandList> pCmdList, shared_ptr<TextureWrapper> tex, UINT idx, UINT mipLevel);
		void									UploadComputeUav(ComPtr<ID3D12GraphicsCommandList> pCmdList, shared_ptr<TextureWrapper> tex, UINT idx);
		void									UploadComputeUavMip(ComPtr<ID3D12GraphicsCommandList> pCmdList, shared_ptr<TextureWrapper> tex, UINT idx, UINT mipLevel);
		void									SetImGuiImage(shared_ptr<TextureWrapper> tex, UINT width, UINT height);
		void									GetDimension(shared_ptr<TextureWrapper> tex, OUT UINT& width, OUT UINT& height);
	};


	extern shared_ptr<TextureManager> g_pTextureManager;
}


