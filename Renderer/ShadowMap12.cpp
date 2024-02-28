#include "ShadowMap12.h"
#include "DescriptorHeapManager.h"
#include "D3D12.h"
namespace wilson
{
	ShadowMap12::ShadowMap12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList,
		DescriptorHeapManager* pDescriptorHeapManager,
		const UINT width, const UINT height, const UINT cascadeLevel, 
		const UINT dirLightCap, const UINT cubeLightCap, const UINT spotLightCap)
	{

		m_dir12Tex.resize(dirLightCap);
		m_dirDebug12Tex.resize(dirLightCap);
		m_dirDebug12RTVs.resize(dirLightCap);

		m_cube12Tex.resize(cubeLightCap);
		m_cubeDebug12Tex.resize(cubeLightCap);
		m_cubeDebug12RTVs.resize(cubeLightCap);

		m_spot12Tex.resize(spotLightCap);
		m_spotDebug12Tex.resize(spotLightCap);
		m_spotDebug12RTVs.resize(spotLightCap);

		m_dir12SRVs.resize(dirLightCap);
		m_dirDebug12SRVs.resize(dirLightCap);

		m_cube12SRVs.resize(cubeLightCap);
		m_cubeDebug12SRVs.resize(cubeLightCap);

		m_spot12SRVs.resize(spotLightCap);
		m_spotDebug12SRVs.resize(spotLightCap);
		//CubeMap==6 
		m_debug12Tex.resize(max(cascadeLevel,6));
		m_debug12SRVs.resize(max(cascadeLevel, 6));

		m_dir12DSVs.resize(dirLightCap);
		m_cube12DSVs.resize(cubeLightCap);
		m_spot12DSVs.resize(spotLightCap);

		m_rect.right = width;
		m_rect.bottom = height;


		m_width = width;
		m_height = height;

		m_viewport12.TopLeftX = 0.0f;
		m_viewport12.TopLeftY = 0.0f;
		m_viewport12.Width = static_cast<float>(width);
		m_viewport12.Height = static_cast<float>(height);
		m_viewport12.MinDepth = 0.0f;
		m_viewport12.MaxDepth = 1.0f;

		HRESULT hr;
		{
			D3D12_CLEAR_VALUE clearVal = {};
			clearVal.Format = DXGI_FORMAT_D32_FLOAT;
			clearVal.DepthStencil.Depth = 1.0f;
			//Gen Tex
			{
				D3D12_HEAP_PROPERTIES heapProps = {};
				heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
				heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heapProps.CreationNodeMask = 1;
				heapProps.VisibleNodeMask = 1;

				D3D12_RESOURCE_DESC	texDesc = {};
				texDesc.Width = width;
				texDesc.Height = height;
				texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
				texDesc.Alignment = 0;
				texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
				texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				texDesc.DepthOrArraySize = max(cascadeLevel, 6);
				texDesc.MipLevels = 1;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;

				D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
				dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
				dsvDesc.Texture2DArray.ArraySize = texDesc.DepthOrArraySize;

				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
				srvDesc.Texture2DArray.ArraySize = texDesc.DepthOrArraySize;
				srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;




				for (int i = 0; i < m_dir12Tex.size(); ++i)
				{
					D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuHandle = pDescriptorHeapManager->GetCurDsvHandle();
					D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
					D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

					hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
						D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal , IID_PPV_ARGS(&m_dir12Tex[i]));
					assert(SUCCEEDED(hr));
					m_dir12Tex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap:::m_dir12Tex[i]") - 1, "ShadowMap:::m_dir12Tex[i]");

					pDevice->CreateDepthStencilView(m_dir12Tex[i], &dsvDesc, dsvCpuHandle);
					m_dir12DSVs[i] = dsvCpuHandle;
					pDescriptorHeapManager->IncreaseDsvHandleOffset();

					pDevice->CreateShaderResourceView(m_dir12Tex[i], &srvDesc, cbvSrvCpuHandle);
					m_dir12SRVs[i] = cbvSrvGpuHandle;
					pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

				}

				//Gen CubeShadowMap
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
					for (int i = 0; i < m_cube12Tex.size(); ++i)
					{
						D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuHandle = pDescriptorHeapManager->GetCurDsvHandle();
						D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
						D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

						hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_cube12Tex[i]));
						assert(SUCCEEDED(hr));
						m_cube12Tex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
							sizeof("ShadowMap:::m_cube12Tex[i]") - 1, "ShadowMap:::m_cube12Tex[i]");

						pDevice->CreateDepthStencilView(m_cube12Tex[i], &dsvDesc, dsvCpuHandle);
						m_cube12DSVs[i] = dsvCpuHandle;
						pDescriptorHeapManager->IncreaseDsvHandleOffset();

						pDevice->CreateShaderResourceView(m_cube12Tex[i], &srvDesc, cbvSrvCpuHandle);
						m_cube12SRVs[i] = cbvSrvGpuHandle;
						pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
					}
				}

				//Gen SpotShadowMap
				{
					texDesc.DepthOrArraySize = 1;
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

					for (int i = 0; i < m_spot12Tex.size(); ++i)
					{
						D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuHandle = pDescriptorHeapManager->GetCurDsvHandle();
						D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
						D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

						hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_spot12Tex[i]));
						assert(SUCCEEDED(hr));
						m_spot12Tex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
							sizeof("ShadowMap:::m_spot12Tex[i]") - 1, "ShadowMap:::m_spot12Tex[i]");

						pDevice->CreateDepthStencilView(m_spot12Tex[i], &dsvDesc, dsvCpuHandle);
						m_spot12DSVs[i] = dsvCpuHandle;
						pDescriptorHeapManager->IncreaseDsvHandleOffset();

						pDevice->CreateShaderResourceView(m_spot12Tex[i], &srvDesc, cbvSrvCpuHandle);
						m_spot12SRVs[i] = cbvSrvGpuHandle;
						pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
					}
				}


			}
		}

		{	
			D3D12_CLEAR_VALUE clearVal = {};
			clearVal.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			clearVal.Color[0] = 1.0f;
			clearVal.Color[1] = 1.0f;
			clearVal.Color[2] = 1.0f;
			clearVal.Color[3] = 1.0f;

			//Gen DebugTex
			{
				D3D12_HEAP_PROPERTIES heapProps = {};
				heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
				heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heapProps.CreationNodeMask = 1;
				heapProps.VisibleNodeMask = 1;

				D3D12_RESOURCE_DESC	texDesc = {};
				texDesc.Width = width;
				texDesc.Height = height;
				texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
				texDesc.Alignment = 0;
				texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				texDesc.DepthOrArraySize = max(cascadeLevel,6);
				texDesc.MipLevels = 1;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;

				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
				rtvDesc.Format = texDesc.Format;
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Texture2DArray.ArraySize = texDesc.DepthOrArraySize;
				rtvDesc.Texture2DArray.MipSlice = 0;

				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Format = texDesc.Format;
				srvDesc.Texture2DArray.ArraySize = texDesc.DepthOrArraySize;
				srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


				for (int i = 0; i < m_dirDebug12Tex.size(); ++i)
				{
					D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle = pDescriptorHeapManager->GetCurRtvHandle();
					D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
					D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

					hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
						D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_dirDebug12Tex[i]));
					assert(SUCCEEDED(hr));
					m_dirDebug12Tex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap:::m_dirDebug12Tex[i]") - 1, "ShadowMap:::m_dirDebug12Tex[i]");

					pDevice->CreateRenderTargetView(m_dirDebug12Tex[i], &rtvDesc, rtvCpuHandle);
					m_dirDebug12RTVs[i] = rtvCpuHandle;
					pDescriptorHeapManager->IncreaseRtvHandleOffset();

					pDevice->CreateShaderResourceView(m_dirDebug12Tex[i], &srvDesc, cbvSrvCpuHandle);
					m_dirDebug12SRVs[i] = cbvSrvGpuHandle;
					pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
				}

				//Gen CubeShadowMap
				{
					for (int i = 0; i < m_cubeDebug12Tex.size(); ++i)
					{
						D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle = pDescriptorHeapManager->GetCurRtvHandle();
						D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
						D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();


						hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_cubeDebug12Tex[i]));
						assert(SUCCEEDED(hr));
						m_cubeDebug12Tex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
							sizeof("ShadowMap:::m_cubeDebug12Tex[i]") - 1, "ShadowMap:::m_cubeDebug12Tex[i]");


						pDevice->CreateRenderTargetView(m_cubeDebug12Tex[i], &rtvDesc, rtvCpuHandle);
						m_cubeDebug12RTVs[i] = rtvCpuHandle;
						pDescriptorHeapManager->IncreaseRtvHandleOffset();

						pDevice->CreateShaderResourceView(m_cubeDebug12Tex[i], &srvDesc, cbvSrvCpuHandle);
						m_cubeDebug12SRVs[i] = cbvSrvGpuHandle;
						pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
					}
				}

				//Gen SpotShadowMap
				{
					texDesc.DepthOrArraySize = 1;
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

					for (int i = 0; i < m_spotDebug12Tex.size(); ++i)
					{
						D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle = pDescriptorHeapManager->GetCurRtvHandle();
						D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
						D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

						hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_spotDebug12Tex[i]));
						assert(SUCCEEDED(hr));
						m_spotDebug12Tex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
							sizeof("ShadowMap:::m_spotDebug12Tex[i]") - 1, "ShadowMap:::m_spotDebug12Tex[i]");


						pDevice->CreateRenderTargetView(m_spotDebug12Tex[i], &rtvDesc, rtvCpuHandle);
						m_spotDebug12RTVs[i] = rtvCpuHandle;
						pDescriptorHeapManager->IncreaseRtvHandleOffset();

						pDevice->CreateShaderResourceView(m_spotDebug12Tex[i], &srvDesc, cbvSrvCpuHandle);
						m_spotDebug12SRVs[i] = cbvSrvGpuHandle;
						pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
					}
				}

				//Gen DebugTex
				{
					for (int i = 0; i < m_debug12Tex.size(); ++i)
					{
						D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
						D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

						hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_debug12Tex[i]));
						assert(SUCCEEDED(hr));
						m_debug12Tex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
							sizeof("ShadowMap:::m_debug12Tex[i]") - 1, "ShadowMap:::m_debug12Tex[i]");


						pDevice->CreateShaderResourceView(m_debug12Tex[i], &srvDesc, cbvSrvCpuHandle);
						m_debug12SRVs[i] = cbvSrvGpuHandle;
						pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

					}
				}

			}
		}
		//Gen Samplers
		{

			D3D12_SAMPLER_DESC ssDesc = {};
			ssDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			ssDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			ssDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			ssDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			ssDesc.BorderColor[0] = 1.0f;
			ssDesc.BorderColor[1] = 1.0f;
			ssDesc.BorderColor[2] = 1.0f;
			ssDesc.BorderColor[3] = 1.0f;
			ssDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;

			{
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = pDescriptorHeapManager->GetCurSamplerCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = pDescriptorHeapManager->GetCurSamplerGpuHandle();

				pDevice->CreateSampler(&ssDesc, samplerCpuHandle);
				m_dirShadowSSV = samplerGpuHandle;
				pDescriptorHeapManager->IncreaseSamplerHandleOffset();
			}

			{
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = pDescriptorHeapManager->GetCurSamplerCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = pDescriptorHeapManager->GetCurSamplerGpuHandle();
				pDevice->CreateSampler(&ssDesc, samplerCpuHandle);
				m_cubeShadowSSV = samplerGpuHandle;
				pDescriptorHeapManager->IncreaseSamplerHandleOffset();
			}
			
		}

	}

	void ShadowMap12::BindDirDSV(ID3D12GraphicsCommandList* pCommandlist, const UINT i)
	{	
		pCommandlist->OMSetRenderTargets(1, &m_dirDebug12RTVs[i], TRUE, &m_dir12DSVs[i]);
	}

	void ShadowMap12::BindCubeDSV(ID3D12GraphicsCommandList* pCommandlist, const UINT i)
	{
		pCommandlist->OMSetRenderTargets(1, &m_cubeDebug12RTVs[i], TRUE, &m_cube12DSVs[i]);
	}

	void ShadowMap12::BindSpotDSV(ID3D12GraphicsCommandList* pCommandlist, const UINT i)
	{
		pCommandlist->OMSetRenderTargets(1, &m_spotDebug12RTVs[i], TRUE, &m_spot12DSVs[i]);
	}

	void ShadowMap12::ClearDSV(ID3D12GraphicsCommandList* pCommandlist, UINT litCounts[])
	{
		for (int i = 0; i < litCounts[0]; ++i)
		{
			pCommandlist->ClearDepthStencilView(m_dir12DSVs[i], D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		}

		for (int i = 0; i < litCounts[1]; ++i)
		{
			pCommandlist->ClearDepthStencilView(m_cube12DSVs[i], D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		}

		for (int i = 0; i < litCounts[2]; ++i)
		{
			pCommandlist->ClearDepthStencilView(m_spot12DSVs[i], D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		}
	}

	void ShadowMap12::ClearRTV(ID3D12GraphicsCommandList* pCommandlist, UINT litCounts[])
	{	

		for (int i = 0; i < litCounts[0]; ++i)
		{
			pCommandlist->ClearRenderTargetView(m_dirDebug12RTVs[i], m_clear, 0, nullptr);
		}

		for (int i = 0; i < litCounts[1]; ++i)
		{
			pCommandlist->ClearRenderTargetView(m_cubeDebug12RTVs[i], m_clear, 0, nullptr);
		}

		for (int i = 0; i < litCounts[2]; ++i)
		{
			pCommandlist->ClearRenderTargetView(m_spotDebug12RTVs[i], m_clear, 0, nullptr);
		}
	}

	void ShadowMap12::SetResourceBarrier(ID3D12GraphicsCommandList* pCommandlist, UINT litCnts[], 
		D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, bool bRTV)
	{
		UINT litCntSum = litCnts[0] + litCnts[1] + litCnts[2];
		std::vector<D3D12_RESOURCE_BARRIER> barriers(litCntSum);
		int j = 0;
		if (bRTV)
		{	
			for (int i = 0; i < litCnts[0]; ++i)
			{	
				barriers[j++] = D3D12::CreateResourceBarrier(m_dirDebug12Tex[i], beforeState, afterState);
			}

			for (int i = 0; i < litCnts[1]; ++i)
			{
				barriers[j++] = D3D12::CreateResourceBarrier(m_cubeDebug12Tex[i], beforeState, afterState);
			}

			for (int i = 0; i < litCnts[2]; ++i)
			{
				barriers[j++] = D3D12::CreateResourceBarrier(m_spotDebug12Tex[i], beforeState, afterState);
			}
		}
		else
		{
			for (int i = 0; i < litCnts[0]; ++i)
			{
				barriers[j++] = D3D12::CreateResourceBarrier(m_dir12Tex[i], beforeState, afterState);
			}

			for (int i = 0; i < litCnts[1]; ++i)
			{
				barriers[j++] = D3D12::CreateResourceBarrier(m_cube12Tex[i], beforeState, afterState);
			}

			for (int i = 0; i < litCnts[2]; ++i)
			{
				barriers[j++] = D3D12::CreateResourceBarrier(m_spot12Tex[i], beforeState, afterState);
			}
		}

		pCommandlist->ResourceBarrier(litCntSum, barriers.data());
	}

	D3D12_GPU_DESCRIPTOR_HANDLE* ShadowMap12::GetDirDebugSRV(ID3D12GraphicsCommandList* pCommandlist, UINT i, UINT lod)
	{	
		D3D12_RESOURCE_BARRIER srvToCopySrc = {};
		srvToCopySrc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		srvToCopySrc.Transition.pResource = m_dirDebug12Tex[i];
		srvToCopySrc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		srvToCopySrc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		srvToCopySrc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

		D3D12_RESOURCE_BARRIER srvToCopyDst = {};
		srvToCopyDst.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		srvToCopyDst.Transition.pResource = m_debug12Tex[lod];
		srvToCopyDst.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		srvToCopyDst.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		srvToCopyDst.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

		D3D12_RESOURCE_BARRIER barriers[2];
		barriers[0] = srvToCopyDst;
		barriers[1] = srvToCopySrc;
		pCommandlist->ResourceBarrier(2, barriers);

		D3D12_RESOURCE_BARRIER copySrcToSrv = {};
		copySrcToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		copySrcToSrv.Transition.pResource = m_dirDebug12Tex[i];
		copySrcToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		copySrcToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		copySrcToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_RESOURCE_BARRIER copyDstToSrv = {};
		copyDstToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		copyDstToSrv.Transition.pResource = m_debug12Tex[lod];
		copyDstToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		copyDstToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		copyDstToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_dirDebug12Tex[i];
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = lod;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = m_debug12Tex[lod];
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		pCommandlist->CopyTextureRegion(&dst, 0, 0, 0,
			&src, NULL);

		barriers[0] = copyDstToSrv;
		barriers[1] = copySrcToSrv;
		pCommandlist->ResourceBarrier(2, barriers);
		return &m_debug12SRVs[lod];
	}

	D3D12_GPU_DESCRIPTOR_HANDLE* ShadowMap12::GetCubeDebugSRV(ID3D12GraphicsCommandList* pCommandlist, UINT i, UINT face)
	{
		D3D12_RESOURCE_BARRIER srvToCopySrc = {};
		srvToCopySrc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		srvToCopySrc.Transition.pResource = m_cubeDebug12Tex[i];
		srvToCopySrc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		srvToCopySrc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		srvToCopySrc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

		D3D12_RESOURCE_BARRIER srvToCopyDst = {};
		srvToCopyDst.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		srvToCopyDst.Transition.pResource = m_debug12Tex[face];
		srvToCopyDst.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		srvToCopyDst.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		srvToCopyDst.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

		D3D12_RESOURCE_BARRIER barriers[2];
		barriers[0] = srvToCopyDst;
		barriers[1] = srvToCopySrc;
		pCommandlist->ResourceBarrier(2, barriers);

		D3D12_RESOURCE_BARRIER copySrcToSrv = {};
		copySrcToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		copySrcToSrv.Transition.pResource = m_cubeDebug12Tex[i];
		copySrcToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		copySrcToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		copySrcToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_RESOURCE_BARRIER copyDstToSrv = {};
		copyDstToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		copyDstToSrv.Transition.pResource = m_debug12Tex[face];
		copyDstToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		copyDstToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		copyDstToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_cubeDebug12Tex[i];
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = face;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = m_debug12Tex[face];
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		pCommandlist->CopyTextureRegion(&dst, 0, 0, 0,
			&src, NULL);
		barriers[0] = copyDstToSrv;
		barriers[1] = copySrcToSrv;
		pCommandlist->ResourceBarrier(2, barriers);

		return &m_debug12SRVs[face];
	}

	D3D12_GPU_DESCRIPTOR_HANDLE* ShadowMap12::GetSpotDebugSRV(ID3D12GraphicsCommandList* pCommandlist, UINT i)
	{
		D3D12_RESOURCE_BARRIER srvToCopySrc = {};
		srvToCopySrc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		srvToCopySrc.Transition.pResource = m_spotDebug12Tex[i];
		srvToCopySrc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		srvToCopySrc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		srvToCopySrc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

		D3D12_RESOURCE_BARRIER srvToCopyDst = {};
		srvToCopyDst.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		srvToCopyDst.Transition.pResource = m_debug12Tex[0];
		srvToCopyDst.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		srvToCopyDst.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		srvToCopyDst.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

		D3D12_RESOURCE_BARRIER barriers[2];
		barriers[0] = srvToCopyDst;
		barriers[1] = srvToCopySrc;
		pCommandlist->ResourceBarrier(2, barriers);

		D3D12_RESOURCE_BARRIER copySrcToSrv = {};
		copySrcToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		copySrcToSrv.Transition.pResource = m_spotDebug12Tex[i];
		copySrcToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		copySrcToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		copySrcToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_RESOURCE_BARRIER copyDstToSrv = {};
		copyDstToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		copyDstToSrv.Transition.pResource = m_debug12Tex[0];
		copyDstToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		copyDstToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		copyDstToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_spotDebug12Tex[i];
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = m_debug12Tex[0];
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		pCommandlist->CopyTextureRegion(&dst, 0, 0, 0,
			&src, NULL);

		barriers[0] = copyDstToSrv;
		barriers[1] = copySrcToSrv;
		pCommandlist->ResourceBarrier(2, barriers);

		return &m_debug12SRVs[0];
	}


	ShadowMap12::~ShadowMap12()
	{
		for (int i = 0; i < m_dir12Tex.size(); ++i)
		{
			if (m_dir12Tex[i] != nullptr)
			{
				m_dir12Tex[i]->Release();
				m_dir12Tex[i] = nullptr;
			}

			if (m_dirDebug12Tex[i] != nullptr)
			{
				m_dirDebug12Tex[i]->Release();
				m_dirDebug12Tex[i] = nullptr;
			}

		}

		for (int i = 0; i < m_cube12Tex.size(); ++i)
		{
			if (m_cube12Tex[i] != nullptr)
			{
				m_cube12Tex[i]->Release();
				m_cube12Tex[i] = nullptr;
			}
			if (m_cubeDebug12Tex[i] != nullptr)
			{
				m_cubeDebug12Tex[i]->Release();
				m_cubeDebug12Tex[i] = nullptr;
			}

		}

		for (int i = 0; i < m_spot12Tex.size(); ++i)
		{
			if (m_spot12Tex[i] != nullptr)
			{
				m_spot12Tex[i]->Release();
				m_spot12Tex[i] = nullptr;
			}

			if (m_spotDebug12Tex[i] != nullptr)
			{
				m_spotDebug12Tex[i]->Release();
				m_spotDebug12Tex[i] = nullptr;
			}

		}

		for (int i = 0; i < m_debug12Tex.size(); ++i)
		{
			if (m_debug12Tex[i] != nullptr)
			{
				m_debug12Tex[i]->Release();
				m_debug12Tex[i] = nullptr;
			}
		}


		m_dir12Tex.clear();
		m_dir12DSVs.clear();
		m_dir12SRVs.clear();
		m_cube12Tex.clear();
		m_cube12DSVs.clear();
		m_cube12SRVs.clear();
		m_spot12Tex.clear();
		m_spot12DSVs.clear();
		m_spot12SRVs.clear();
	}

}