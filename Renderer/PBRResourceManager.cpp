#define STB_IMAGE_IMPLEMENTATION
#include "PBRResourceManager.h"
#include "stb_image.h"
#include "CommandListWrapperPool.h"
#include "D3D12.h"
#include "HeapManager.h"
#include "PSOManager.h"
#include "SamplerManager.h"
#include "ResourceBarrierHandler.h"
#include "TextureManager.h"
namespace wilson
{
	
	PBRResourceManager::PBRResourceManager(shared_ptr<PSOManager> psoManager)
	{
		//GenPBR Resources	
		{
			m_pBrdfTex = g_pTextureManager->CreateTexture2D(_SHADOWMAP_SIZE, _SHADOWMAP_SIZE, 1, 1, (BYTE)0b0011,
				DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			SET_PRIVATE_DATA(m_pBrdfTex->tex);

			m_pSkyBoxTex = g_pTextureManager->CreateTexture2D(_SHADOWMAP_SIZE, _SHADOWMAP_SIZE, _CUBE_FACES, 1, (BYTE)0b101010,
				DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			SET_PRIVATE_DATA(m_pSkyBoxTex->tex);

			m_pDiffIrradianceTex = g_pTextureManager->CreateTexture2D(_DIFFIRRAD_WIDTH, _DIFFIRRAD_WIDTH, _CUBE_FACES, 1, (BYTE)0b101010,
				DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			SET_PRIVATE_DATA(m_pDiffIrradianceTex->tex);


			m_pPrefilterTex = g_pTextureManager->CreateTexture2D(_PREFILTER_WIDTH, _PREFILTER_HEIGHT, _CUBE_FACES, _PREFILTER_MIP_LEVELS, (BYTE)0b101010,
				DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			SET_PRIVATE_DATA(m_pPrefilterTex->tex);

			m_pGenMipUavTex = g_pTextureManager->CreateTexture2D(_PREFILTER_WIDTH, _PREFILTER_HEIGHT, _CUBE_FACES, _PREFILTER_MIP_LEVELS, (BYTE)0b110001,
				DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			SET_PRIVATE_DATA(m_pGenMipUavTex->tex);

		}
		//Gen Cube Buffer
		{
			XMFLOAT3 vertices[] = {
			 {XMFLOAT3(-1.0f,  1.0f, -1.0f)},//front-upper-left  0
			 {XMFLOAT3(1.0f,   1.0f, -1.0f)},//front-upper-right 1
			 {XMFLOAT3(1.0f,  -1.0f, -1.0f)},//front-down-right  2
			 {XMFLOAT3(-1.0f, -1.0f, -1.0f)},//front-down-left   3

			 {XMFLOAT3(-1.0f,  1.0f, 1.0f)},//back-upper-left   4
			 {XMFLOAT3(1.0f,   1.0f, 1.0f)},//back-upper-right  5
			 {XMFLOAT3(1.0f,  -1.0f, 1.0f)},//back-down-right   6
			 {XMFLOAT3(-1.0f, -1.0f, 1.0f)} };//back-down-left   7
			UINT skyVbSize = sizeof(vertices);


			g_pHeapManager->AllocateVertexData(reinterpret_cast<UINT8*>(vertices), skyVbSize);
			m_skyBoxVbv = g_pHeapManager->GetVbv(skyVbSize, sizeof(XMFLOAT3));

			//시계방향 감은 면이  frontface
			unsigned long indices[] = {
				//front
				0,2,1,
				0,3,2,
				//back
				4,5,6,
				4,6,7,
				//left
				0,4,7,
				0,7,3,
				//right
				5,1,2,
				5,2,6,
				//top
				5,4,0,
				5,0,1,
				//bottom
				7,6,2,
				7,2,3
			};
			UINT skyIbSize = sizeof(indices);
			g_pHeapManager->AllocateIndexData(reinterpret_cast<UINT8*>(indices), skyIbSize);
			m_skyBoxIbv = g_pHeapManager->GetIbv(skyIbSize, 0);
			UINT idx = g_pHeapManager->GetIbHeapOffset();
			idx /= _IB_HEAP_SIZE;
			UINT64 curBlockOffset = g_pHeapManager->GetIbBlockOffset(idx);
			g_pHeapManager->SetIbBlockOffset(idx, curBlockOffset + m_skyBoxIbv.SizeInBytes);
		}

		m_equirect2CubeCbKey	= g_pHeapManager->AllocateCb(sizeof(XMMATRIX) * _CUBE_FACES);
		m_skyMatrixCbKey		= g_pHeapManager->AllocateCb(sizeof(XMMATRIX));
		m_roughnessCbKey		= g_pHeapManager->AllocateCb(sizeof(XMVECTOR));
		m_resolutionCbKey		= g_pHeapManager->AllocateCb(sizeof(UINT) * 2);
		{
			m_diffIrradViewport.MinDepth = 0.0f;
			m_diffIrradViewport.MaxDepth = 1.0f;
			m_diffIrradViewport.TopLeftX = 0.0f;
			m_diffIrradViewport.TopLeftY = 0.0f;
			m_diffIrradViewport.Width = _DIFFIRRAD_WIDTH;
			m_diffIrradViewport.Height = _DIFFIRRAD_HEIGHT;

			m_diffIrradRect.left = 0;
			m_diffIrradRect.top = 0;
			m_diffIrradRect.right = m_diffIrradViewport.Width;
			m_diffIrradRect.bottom = m_diffIrradViewport.Height;

			m_prefilterViewport.MinDepth = 0.0f;
			m_prefilterViewport.MaxDepth = 1.0f;
			m_prefilterViewport.TopLeftX = 0.0f;
			m_prefilterViewport.TopLeftY = 0.0f;
			m_prefilterViewport.Width = _PREFILTER_WIDTH;
			m_prefilterViewport.Height = _PREFILTER_HEIGHT;

			m_prefilterRect.left = 0;
			m_prefilterRect.top = 0;
			m_prefilterRect.right = m_prefilterViewport.Width;
			m_prefilterRect.bottom = m_prefilterViewport.Height;



			m_skyboxViewport.MinDepth = 0.0f;
			m_skyboxViewport.MaxDepth = 1.0f;
			m_skyboxViewport.TopLeftX = 0.0f;
			m_skyboxViewport.TopLeftY = 0.0f;
			m_skyboxViewport.Width = _SHADOWMAP_SIZE;
			m_skyboxViewport.Height = _SHADOWMAP_SIZE;

			m_skyboxRect.left = 0;
			m_skyboxRect.top = 0;
			m_skyboxRect.right = m_skyboxViewport.Width;
			m_skyboxRect.bottom = m_skyboxViewport.Height;
		}

		Init(psoManager);
	}
	PBRResourceManager::~PBRResourceManager()
	{
		if (m_pBrdfTex != nullptr)
		{
			m_pBrdfTex = nullptr;
		}

		if (m_pSkyBoxTex != nullptr)
		{
			m_pSkyBoxTex = nullptr;
		}

		if (m_pDiffIrradianceTex != nullptr)
		{
			m_pDiffIrradianceTex = nullptr;
		}

		if (m_pPrefilterTex != nullptr)
		{
			m_pPrefilterTex = nullptr;
		}

		if (m_pGenMipUavTex != nullptr)
		{
			m_pGenMipUavTex = nullptr;
		}
	}
	bool PBRResourceManager::CreateEquirentangularMap(const char* pPath)
	{
		HRESULT hr;
		int width, height, nrComponents;
		//8perChannel->32perChannel
		float* pData = stbi_loadf(pPath,
			&width, &height, &nrComponents, STBI_rgb_alpha);
		if (pData)
		{
			D3D12_RESOURCE_DESC	texDesc = {};
			texDesc.Width = width;
			texDesc.Height = height;
			texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			texDesc.Alignment = 0;
			texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			texDesc.DepthOrArraySize = 1;
			texDesc.MipLevels = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = texDesc.Format;
			srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			DestroyHdr();
			

			m_pHdrTex = g_pTextureManager->CreateTexture2D(width, height, 1, 1, (BYTE)(0b10),
				DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			SET_PRIVATE_DATA(m_pHdrTex->tex);

			shared_ptr<CommandListWrapper> pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
			ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();

			g_pHeapManager->SetDescriptorHeaps(pCmdList);
			g_pTextureManager->UploadTexThroughCB(texDesc, sizeof(XMVECTOR) * width, reinterpret_cast<UINT8*>(pData),
				m_pHdrTex->tex, m_pHdrTex->cb, pCmdList);
			SET_PRIVATE_DATA(m_pHdrTex->cb);

			g_pResourceBarrierHandler->Push(
				g_pTextureManager->CreateBarrier(m_pHdrTex, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
			g_pResourceBarrierHandler->Flush(pCmdList);


			g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
			g_pGraphicsCmdListPool->Push(pCmdListWrapper);
			return true;
		}
		else
		{
			return false;
		}
	}
	void PBRResourceManager::ConvertEquirectagular2Cube(shared_ptr<PSOManager> psoManager)
	{

		UINT stride;
		UINT offset = 0;

		XMMATRIX capProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(90.0f),
			1.0F, 0.1f, 10.0f);
		XMVECTOR eyePos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMMATRIX capView[_CUBE_FACES] =
		{
			XMMatrixLookAtLH(eyePos, XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
			XMMatrixLookAtLH(eyePos, XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
			XMMatrixLookAtLH(eyePos, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f)),
			XMMatrixLookAtLH(eyePos, XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)),
			XMMatrixLookAtLH(eyePos, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
			XMMatrixLookAtLH(eyePos, XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
		};
		XMMATRIX pMatrix[_CUBE_FACES];
		for (int i = 0; i < _CUBE_FACES; ++i)
		{
			pMatrix[i] = XMMatrixMultiplyTranspose(capView[i], capProj);

		}
		g_pHeapManager->CopyDataToCb(m_equirect2CubeCbKey, sizeof(XMMATRIX) * _CUBE_FACES, pMatrix);

	
		


		shared_ptr<CommandListWrapper> pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
		ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();
		
		psoManager->SetEquirect2Cube(pCmdList);
		g_pHeapManager->SetDescriptorHeaps(pCmdList);


		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pSkyBoxTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		g_pResourceBarrierHandler->Flush(pCmdList);

		g_pTextureManager->UploadGraphicsSrv(pCmdList, m_pHdrTex, E_TO_UINT(eEquirect2CubeRP::psTex));
		g_pSamplerManager->UploadWrapSampler(pCmdList, E_TO_UINT(eEquirect2CubeRP::psSampler), false);
		g_pHeapManager->UploadGraphicsCb(m_equirect2CubeCbKey, E_TO_UINT(eEquirect2CubeRP::gsCb), pCmdList);
		UploadSkyBoxIA(pCmdList);
		pCmdList->RSSetViewports(1, &m_skyboxViewport);
		pCmdList->RSSetScissorRects(1, &m_skyboxRect);
		pCmdList->OMSetRenderTargets(1, &m_pSkyBoxTex->rtv, TRUE, nullptr);
		pCmdList->DrawIndexedInstanced(_CUBE_IDX_COUNT, 1, 0, 0, 0);


		g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
		g_pGraphicsCmdListPool->Push(pCmdListWrapper);

	}
	void PBRResourceManager::CreateDiffuseIrradianceMap(shared_ptr<PSOManager> psoManager)
	{
		shared_ptr<CommandListWrapper> pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
		ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();

		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_pSkyBoxTex->tex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_pDiffIrradianceTex->tex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		g_pResourceBarrierHandler->Flush(pCmdList);

		g_pHeapManager->SetDescriptorHeaps(pCmdList);
		psoManager->SetDiffuseIrrad(pCmdList);
		pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pCmdList->IASetVertexBuffers(0, 1, &m_skyBoxVbv);
		pCmdList->IASetIndexBuffer(&m_skyBoxIbv);
		g_pTextureManager->UploadGraphicsSrv(pCmdList, m_pSkyBoxTex, E_TO_UINT(eDiffuseIrraidianceRP::psTex));
		g_pSamplerManager->UploadClampLinearSampler(pCmdList, E_TO_UINT(eDiffuseIrraidianceRP::psSampler), false);
		g_pHeapManager->UploadGraphicsCb(m_equirect2CubeCbKey, E_TO_UINT(eDiffuseIrraidianceRP::gsCb), pCmdList);
		pCmdList->RSSetViewports(1, &m_diffIrradViewport);
		pCmdList->RSSetScissorRects(1, &m_diffIrradRect);
		pCmdList->OMSetRenderTargets(1, &m_pDiffIrradianceTex->rtv, TRUE, nullptr);
		pCmdList->DrawIndexedInstanced(_CUBE_IDX_COUNT, 1, 0, 0, 0);


		g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
		g_pGraphicsCmdListPool->Push(pCmdListWrapper);

	}
	void PBRResourceManager::CreatePrefileterMap(shared_ptr<PSOManager> psoManager)
	{
		FLOAT r = 0;
		g_pHeapManager->CopyDataToCb(m_roughnessCbKey, sizeof(FLOAT), &r);

		shared_ptr<CommandListWrapper> pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
		ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();

		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_pDiffIrradianceTex->tex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_pPrefilterTex->tex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		g_pResourceBarrierHandler->Flush(pCmdList);

		psoManager->SetPrefilter(pCmdList);
		g_pHeapManager->SetDescriptorHeaps(pCmdList);
		UploadSkyBoxIA(pCmdList);
		g_pTextureManager	->UploadGraphicsSrv(pCmdList, m_pSkyBoxTex, E_TO_UINT(ePrefilterRP::psTex));
		g_pSamplerManager	->UploadClampLinearSampler(pCmdList, E_TO_UINT(ePrefilterRP::psSampler), false);
		g_pHeapManager		->UploadGraphicsCb(m_equirect2CubeCbKey, E_TO_UINT(ePrefilterRP::gsCb), pCmdList);
		g_pHeapManager		->UploadGraphicsCb(m_roughnessCbKey, E_TO_UINT(ePrefilterRP::psCb), pCmdList);
		pCmdList->RSSetViewports(1, &m_prefilterViewport);
		pCmdList->RSSetScissorRects(1, &m_prefilterRect);
		pCmdList->OMSetRenderTargets(1, &m_pPrefilterTex->rtv, TRUE, nullptr);
		pCmdList->DrawIndexedInstanced(_CUBE_IDX_COUNT, 1, 0, 0, 0);


		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_pPrefilterTex->tex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		g_pResourceBarrierHandler->Flush(pCmdList);


		g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
		g_pGraphicsCmdListPool->Push(pCmdListWrapper);
	}
	void PBRResourceManager::CreateMipMap(shared_ptr<PSOManager> psoManager)
	{

		HRESULT hr;
	
		//Gen Null UAVs
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Format = m_pPrefilterTex->tex->GetDesc().Format;
		uavDesc.Texture2DArray.ArraySize = _CUBE_FACES;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.pResource = m_pPrefilterTex->tex.Get();

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.pResource = m_pGenMipUavTex->tex.Get();


		UINT8* pMipCB;
		D3D12_RANGE readRange = {};

		for (int i = 1; i < _PREFILTER_MIP_LEVELS; ++i)
		{
			
			{
				shared_ptr<CommandListWrapper> pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
				ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();

				g_pResourceBarrierHandler->Push(
					ResourceBarrierHandler::CreateResourceBarrier(m_pPrefilterTex->tex, 
						D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
				g_pResourceBarrierHandler->Flush(pCmdList);

				FLOAT dimension[2] = { _PREFILTER_WIDTH >> i, _PREFILTER_HEIGHT >> i };
				g_pHeapManager->CopyDataToCb(m_resolutionCbKey, sizeof(dimension), dimension);

				psoManager->SetCubeMipGen(pCmdList);
				g_pHeapManager->SetDescriptorHeaps(pCmdList);
				g_pTextureManager->UploadComputeSrv(pCmdList, m_pPrefilterTex, E_TO_UINT(eGenMipRP::csTex));
				pCmdList->SetComputeRootDescriptorTable(E_TO_UINT(eGenMipRP::csUAV), m_pGenMipUavTex->uavs[i]);
				g_pSamplerManager->UploadClampLinearSampler(pCmdList, E_TO_UINT(eGenMipRP::csSampler), true);
				g_pHeapManager->UploadComputeCb(m_resolutionCbKey, E_TO_UINT(eGenMipRP::csCb), pCmdList);
				pCmdList->Dispatch(dimension[0], dimension[1], _CUBE_FACES);//tex Dimesion


				g_pResourceBarrierHandler->Push(
					ResourceBarrierHandler::CreateResourceBarrier(m_pPrefilterTex->tex, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
				g_pResourceBarrierHandler->Push(
					ResourceBarrierHandler::CreateResourceBarrier(m_pGenMipUavTex->tex, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
				g_pResourceBarrierHandler->Flush(pCmdList);
	
				g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
				g_pGraphicsCmdListPool->Push(pCmdListWrapper);
			}

			{
				shared_ptr<CommandListWrapper> pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
				ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();

				for (int j = 0; j < _CUBE_FACES; ++j)
				{
					dst.SubresourceIndex = i + (_PREFILTER_MIP_LEVELS * j);
					src.SubresourceIndex = i + (_PREFILTER_MIP_LEVELS * j);
					pCmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
				}


				g_pResourceBarrierHandler->Push(
					ResourceBarrierHandler::CreateResourceBarrier(m_pPrefilterTex->tex, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
				g_pResourceBarrierHandler->Push(
					ResourceBarrierHandler::CreateResourceBarrier(m_pGenMipUavTex->tex, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
				g_pResourceBarrierHandler->Flush(pCmdList);

				g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
				g_pGraphicsCmdListPool->Push(pCmdListWrapper);
			}


		}


		//https://learn.microsoft.com/en-us/windows/win32/direct3d12/subresources

	}
	void PBRResourceManager::CreateBrdfMap(shared_ptr<PSOManager> psoManager)
	{
		//Gen BRDFMap
		
		shared_ptr<CommandListWrapper>		 pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
		ComPtr<ID3D12GraphicsCommandList>			pCmdList = pCmdListWrapper->GetCmdList();
		FLOAT dimension[2] = { _SHADOWMAP_SIZE, _SHADOWMAP_SIZE };
		g_pHeapManager			->CopyDataToCb(m_resolutionCbKey, sizeof(dimension), dimension);

		psoManager				->SetBrdf(pCmdList);
		g_pHeapManager			->SetDescriptorHeaps(pCmdList);
		g_pTextureManager		->UploadComputeUav(pCmdList, m_pBrdfTex, 0);
		g_pHeapManager			->UploadComputeCb(m_resolutionCbKey, E_TO_UINT(eBrdfRP::dimension), pCmdList);
		pCmdList				->Dispatch(_SHADOWMAP_SIZE, _SHADOWMAP_SIZE,1);

		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pBrdfTex, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		g_pResourceBarrierHandler->Flush(pCmdList);
		g_pGraphicsCmdListPool	->Execute(pCmdListWrapper);
		g_pGraphicsCmdListPool	->Push(pCmdListWrapper);

	}
	void PBRResourceManager::DestroyHdr()
	{
		if (m_pHdrTex != nullptr)
		{
			m_pHdrTex = nullptr;
		}
	}
	void PBRResourceManager::Init(shared_ptr<PSOManager> psoManager)
	{
		bool result =
			CreateEquirentangularMap(".\\Assets\\Models\\FBX\\Bistro_v5_2\\Bistro_v5_2\\san_giuseppe_bridge_4k.hdr");
		if (result)
		{
			ConvertEquirectagular2Cube(psoManager);
			CreateDiffuseIrradianceMap(psoManager);
			CreatePrefileterMap(psoManager);
			CreateMipMap(psoManager);
			CreateBrdfMap(psoManager);

		}
	}
	void PBRResourceManager::SetSkyBoxPass(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		UploadSkyBoxIA(pCmdList);

		g_pHeapManager->UploadGraphicsCb(m_skyMatrixCbKey, 0, pCmdList);
		g_pTextureManager->UploadGraphicsSrv(pCmdList, m_pSkyBoxTex, E_TO_UINT(eSkyboxRP::psSkybox));
	}
	void PBRResourceManager::SetLightingPass(ComPtr<ID3D12GraphicsCommandList>pCmdList)
	{
		g_pTextureManager->UploadGraphicsSrv(pCmdList, m_pDiffIrradianceTex, E_TO_UINT(ePbrLightRP::psIrradiance));
		g_pTextureManager->UploadGraphicsSrv(pCmdList, m_pPrefilterTex, E_TO_UINT(ePbrLightRP::psPrefilter));
		g_pTextureManager->UploadGraphicsSrv(pCmdList, m_pBrdfTex, E_TO_UINT(ePbrLightRP::psBrdf));

	}
	void PBRResourceManager::UpdateSkyBoxMat(shared_ptr<WVPMatrices> pWvpMatrices)
	{
		XMMATRIX wvpMat = XMMatrixMultiplyTranspose(pWvpMatrices->vMat, pWvpMatrices->pMat);

		g_pHeapManager->CopyDataToCb(m_skyMatrixCbKey, sizeof(XMMATRIX), &wvpMat);
	}
	void PBRResourceManager::UploadSkyBoxIA(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pCmdList->IASetVertexBuffers(0, 1, &m_skyBoxVbv);
		pCmdList->IASetIndexBuffer(&m_skyBoxIbv);
	}

}
