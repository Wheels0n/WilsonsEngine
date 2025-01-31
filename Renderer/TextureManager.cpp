#include <dxgi1_4.h>
#include <DirectXTex.h>

#include "D3D12.h"
#include "ResourceBarrierHandler.h"
#include "TextureManager.h"
#include "HeapManager.h"
#include "../ImGui/imgui.h"
namespace wilson
{
	shared_ptr<TextureManager> g_pTextureManager = nullptr;
	
	D3D12_RESOURCE_BARRIER TextureManager::CreateBarrier(shared_ptr<TextureWrapper> pTex, D3D12_RESOURCE_STATES src, D3D12_RESOURCE_STATES dst)
	{
		return ResourceBarrierHandler::CreateResourceBarrier(pTex->tex, src, dst);
	}
	D3D12_RESOURCE_BARRIER TextureManager::CreateBarrierWithMip(shared_ptr<TextureWrapper> pTex, D3D12_RESOURCE_STATES src, D3D12_RESOURCE_STATES dst, UINT mipLevel)
	{
		return ResourceBarrierHandler::CreateResourceBarrier(pTex->tex, src, dst, mipLevel);
	}

	shared_ptr<TextureWrapper> TextureManager::CreateTexture2D(const UINT width, const UINT height, const UINT arraySize, const UINT mipLevels,
		BYTE view, DXGI_FORMAT format, D3D12_RESOURCE_STATES state)
	{
		shared_ptr<TextureWrapper> textureWrapper = make_shared<TextureWrapper>();
		BOOL bCube			= view & (1 << 5);
		BOOL bViewPerMip	= view & (1 << 4);
		BOOL hasRTV			= view & (1 << 3);
		BOOL hasDSV			= view & (1 << 2);
		BOOL hasSRV			= view & (1 << 1);
		BOOL hasUAV			= view & (1 << 0);

		D3D12_RESOURCE_DESC	texDesc = {};
		texDesc.Width								= width;
		texDesc.Height								= height;
		texDesc.Alignment							= 0;
		texDesc.DepthOrArraySize					= arraySize;
		texDesc.MipLevels							= mipLevels;
		texDesc.SampleDesc.Count					= 1;
		texDesc.SampleDesc.Quality					= 0;
		texDesc.Flags								= D3D12_RESOURCE_FLAG_NONE;
		texDesc.Dimension							=  D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Format								= format;
		texDesc.Layout								= D3D12_TEXTURE_LAYOUT_UNKNOWN;

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc		= {};
		rtvDesc.Format								= texDesc.Format;
		rtvDesc.ViewDimension						= arraySize != 1 ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY :D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2DArray.ArraySize			= arraySize;
		rtvDesc.Texture2DArray.MipSlice				= 0;
		rtvDesc.Texture2D.MipSlice					= 0;
		rtvDesc.Texture2D.PlaneSlice				= 0;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc		= {};
		if (arraySize == 1)
		{
			srvDesc.Texture2D.MipLevels					= 1;
			srvDesc.Texture2D.MostDetailedMip			= 0;
			srvDesc.Texture2D.ResourceMinLODClamp		= 0.0f;
		}
		else
		{
			srvDesc.Texture2DArray.ArraySize			= arraySize;
			srvDesc.Texture2DArray.MipLevels			= mipLevels;
			srvDesc.Texture2DArray.MostDetailedMip		= 0;
			srvDesc.Texture2DArray.ResourceMinLODClamp	= 0.0f;
		}
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension						= arraySize!=1 ? D3D12_SRV_DIMENSION_TEXTURE2DARRAY: D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping				= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc	= {};
		uavDesc.ViewDimension						= arraySize != 1 ? D3D12_UAV_DIMENSION_TEXTURE2DARRAY : D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Format								= texDesc.Format;
		uavDesc.Texture2DArray.ArraySize			= arraySize;

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format								= format==DXGI_FORMAT_R32_TYPELESS?DXGI_FORMAT_D32_FLOAT:format;
		dsvDesc.ViewDimension						= arraySize != 1 ? D3D12_DSV_DIMENSION_TEXTURE2DARRAY : D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice					= 0;
		dsvDesc.Texture2DArray.ArraySize			= arraySize;

		if (hasRTV)
		{
			texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			g_pHeapManager->CreateTexture(texDesc, state,
				textureWrapper->tex);

			if (bViewPerMip)
			{
				textureWrapper->rtvs.resize(mipLevels);
				for (int i = 0; i < mipLevels; ++i)
				{
					rtvDesc.Texture2D.MipSlice = i;
					textureWrapper->rtvs[i] = g_pHeapManager->GetRtv(rtvDesc, textureWrapper->tex);
				}
			}
			else
			{
				textureWrapper->rtv =
					g_pHeapManager->GetRtv(rtvDesc, textureWrapper->tex);
			}
			
		}

		if (hasDSV)
		{
			texDesc.Flags		= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			g_pHeapManager->CreateTexture(texDesc, state,
				textureWrapper->tex);

			textureWrapper->dsv = g_pHeapManager->GetDsv(dsvDesc, textureWrapper->tex);
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}

		if (hasUAV)
		{
			texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			g_pHeapManager->CreateTexture(texDesc, state,
				textureWrapper->tex);
			if (bViewPerMip)
			{
				textureWrapper->uavs.resize(mipLevels);
				for (int i = 0; i < mipLevels; ++i)
				{
					if (uavDesc.ViewDimension == D3D12_UAV_DIMENSION_TEXTURE2DARRAY)
					{
						uavDesc.Texture2DArray.MipSlice = i;
					}
					else
					{
						uavDesc.Texture2D.MipSlice		= i;
					}
					
					textureWrapper->uavs[i] = g_pHeapManager->GetUav(uavDesc, textureWrapper->tex);
				}
			}
			
			{
				textureWrapper->uav = g_pHeapManager->GetUav(uavDesc, textureWrapper->tex);
			}
		
		}

		if (hasSRV)
		{
			if(textureWrapper->tex.Get() == nullptr)
			{
				g_pHeapManager->CreateTexture(texDesc, state,
					textureWrapper->tex);
			}
			if (bCube)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srvDesc.TextureCube.MipLevels = mipLevels;
			}

			if (bViewPerMip)
			{
				textureWrapper->srvs.resize(mipLevels);
				for (int i = 0; i < mipLevels; ++i)
				{
					srvDesc.Texture2D.MostDetailedMip = i;
					textureWrapper->srvs[i] = g_pHeapManager->GetSrv(srvDesc, textureWrapper->tex);
				}
			}
			
			{
				textureWrapper->srv = g_pHeapManager->GetSrv(srvDesc, textureWrapper->tex);
			}
			
		}
		return textureWrapper;
	}
	shared_ptr<TextureWrapper> TextureManager::CreateTexture2D(ComPtr<IDXGISwapChain> pSwapChain, UINT i)
	{
		shared_ptr<TextureWrapper> tex			= make_shared<TextureWrapper>();
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle	= g_pHeapManager->GetCurRtvHandle();
		HRESULT hr = pSwapChain->GetBuffer(i, IID_PPV_ARGS(tex->tex.GetAddressOf()));
		assert(SUCCEEDED(hr));
		D3D12::GetDevice()->CreateRenderTargetView(tex->tex.Get(), nullptr, rtvHandle);
		tex->rtv = rtvHandle;
		g_pHeapManager->IncreaseRtvHandleOffset();
		return tex;
	}
	shared_ptr<TextureWrapper> TextureManager::LoadImageWICFile(const WCHAR* pPath, ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		ScratchImage image;
		HRESULT hr = LoadFromWICFile(pPath, WIC_FLAGS_NONE, nullptr, image);
		assert(SUCCEEDED(hr));
		UINT8* pData = image.GetPixels();
		size_t rowPitch;
		size_t slidePitch;
		TexMetadata metadata = image.GetMetadata();
		ComputePitch(metadata.format, metadata.width, metadata.height, rowPitch, slidePitch);

		shared_ptr<TextureWrapper> result = CreateTexture2D(image.GetMetadata().width, image.GetMetadata().height,
			image.GetMetadata().arraySize, image.GetMetadata().mipLevels, (BYTE)0b0010,
			image.GetMetadata().format, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		UploadTexThroughCB(result->tex->GetDesc(), rowPitch, pData, result->tex, result->cb, pCmdList);

		return result;
	}
	shared_ptr<TextureWrapper> TextureManager::LoadImageDDSFile(const WCHAR* pPath, ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		ScratchImage image;
		ScratchImage decompressdImage;
		ScratchImage resizedImage;
		ScratchImage dstImage;
		TexMetadata metadata;

		HRESULT hr = LoadFromDDSFile(pPath, DDS_FLAGS_NONE, nullptr, image);
		metadata = image.GetMetadata();
		UINT8* pData = image.GetPixels();
		if (IsCompressed(metadata.format) &&
			(metadata.width < 4 || metadata.height < 4))
		{
			//Decompress

			hr = Decompress(image.GetImages(), image.GetImageCount(), metadata, DXGI_FORMAT_UNKNOWN, decompressdImage);
			metadata = decompressdImage.GetMetadata();
			//Resize

			hr = Resize(decompressdImage.GetImages(), decompressdImage.GetImageCount(), metadata,
				4, 4, TEX_FILTER_DEFAULT, resizedImage);
			metadata = resizedImage.GetMetadata();
			//Compress

			hr = Compress(resizedImage.GetImages(), resizedImage.GetImageCount(), metadata, image.GetMetadata().format,
				TEX_COMPRESS_DEFAULT, TEX_THRESHOLD_DEFAULT, dstImage);
			metadata = dstImage.GetMetadata();
			pData = dstImage.GetPixels();

		}

		size_t rowPitch;
		size_t slidePitch;
		ComputePitch(metadata.format, metadata.width, metadata.height, rowPitch, slidePitch);


		shared_ptr<TextureWrapper> result = CreateTexture2D(image.GetMetadata().width, image.GetMetadata().height,
			image.GetMetadata().arraySize, image.GetMetadata().mipLevels, (BYTE)0b0010,
			image.GetMetadata().format, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		UploadTexThroughCB(result->tex->GetDesc(), rowPitch, pData, result->tex, result->cb, pCmdList);

		return result;
	}
	void TextureManager::UploadTexThroughCB(const D3D12_RESOURCE_DESC texDesc, const UINT rp, 
		const UINT8* pData, ComPtr<ID3D12Resource>&pDst, ComPtr<ID3D12Resource>& pUploadCb, ComPtr<ID3D12GraphicsCommandList> pCmdList)

	{
		HRESULT hr;

		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(pDst.Get(), 
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
		g_pResourceBarrierHandler->Flush(pCmdList);

		UINT rowPitch = rp;
		UINT height = texDesc.Height;
		//4x4´ÜÀ§¶ó¼­ °¨¾ÈÇØÁà¾ßÇÔ. +3Àº 4ÀÇ¹è¼ö·Î ³ª´²¶³¾îÁö°Ô ÇÏ±â À§ÇÔ.
		if (DirectX::IsCompressed(texDesc.Format))
		{
			height = (height + 3) / 4;
			if (texDesc.Format == DXGI_FORMAT_BC1_UNORM ||
				texDesc.Format == DXGI_FORMAT_BC1_UNORM_SRGB ||
				texDesc.Format == DXGI_FORMAT_BC1_TYPELESS)
			{
				rowPitch = (texDesc.Width + 3) / 4 * 8;
			}
			else
			{
				rowPitch = (texDesc.Width + 3) / 4 * 16;
			}
		}
		UINT64 uploadPitch = (rowPitch + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
		UINT64 uploadSize = uploadPitch * height;
		g_pHeapManager->CreateUploadBuffer(pUploadCb, uploadPitch, uploadSize);


		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
		D3D12_RESOURCE_DESC copytexDesc = texDesc;
		UINT numRows;
		UINT64 rowSizeInByte;
		UINT64 totalBytes;



		ComPtr<ID3D12Device> pDevice = D3D12::GetDevice();
		pDevice->GetCopyableFootprints(&copytexDesc, 0, 1, 0, &footprint, &numRows, &rowSizeInByte, &totalBytes);
		UINT8* pUploadCbPtr;
		D3D12_RANGE readRange = { 0, };
		hr = pUploadCb->Map(0, &readRange, reinterpret_cast<void**>(&pUploadCbPtr));
		assert(SUCCEEDED(hr));
		//Padding ‹š¹®¿¡ memcpyÇÑÁÙ·Î Ã³¸® ºÒ°¡
		for (UINT64 y = 0; y < height; ++y)
		{
			memcpy((void*)(pUploadCbPtr + y * uploadPitch),
				(UINT8*)pData + y * rp, rp);
		}

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.pResource = pDst.Get();

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = pUploadCb.Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint = footprint;

		pCmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		pUploadCb->Unmap(0, 0);
		return;
	}
	void TextureManager::UploadGraphicsSrv(ComPtr<ID3D12GraphicsCommandList> pCmdList, shared_ptr<TextureWrapper> tex, UINT idx)
	{
		pCmdList->SetGraphicsRootDescriptorTable(idx, tex->srv);
	}
	void TextureManager::UploadComputeSrv(ComPtr<ID3D12GraphicsCommandList> pCmdList, shared_ptr<TextureWrapper> tex, UINT idx)
	{
		pCmdList->SetComputeRootDescriptorTable(idx, tex->srv);
	}
	void TextureManager::UploadComputeSrvMip(ComPtr<ID3D12GraphicsCommandList> pCmdList, shared_ptr<TextureWrapper> tex, UINT idx, UINT mipLevel)
	{
		pCmdList->SetComputeRootDescriptorTable(idx, tex->srvs[mipLevel]);
	}
	void TextureManager::UploadComputeUav(ComPtr<ID3D12GraphicsCommandList> pCmdList, shared_ptr<TextureWrapper> tex, UINT idx)
	{
		pCmdList->SetComputeRootDescriptorTable(idx, tex->uav);
	}
	void TextureManager::UploadComputeUavMip(ComPtr<ID3D12GraphicsCommandList> pCmdList, shared_ptr<TextureWrapper> tex, UINT idx, UINT mipLevel)
	{
		pCmdList->SetComputeRootDescriptorTable(idx, tex->uavs[mipLevel]);
	}
	void TextureManager::SetImGuiImage(shared_ptr<TextureWrapper> tex, UINT width, UINT height)
	{
		ImGui::Image((ImTextureID)tex->srv.ptr, ImVec2(width, height));
	}
	void TextureManager::GetDimension(shared_ptr<TextureWrapper> tex, OUT UINT& width, OUT UINT& height)
	{
		D3D12_RESOURCE_DESC desc = tex->tex->GetDesc();
		width =desc.Width;
		height = desc.Height;
	}
}

