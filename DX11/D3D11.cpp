#include <random>
#include "D3D11.h"
#include  "../ImGui/imgui_impl_dx11.h"

namespace wilson
{
	D3D11::D3D11()
	{
		m_pSwapChain = nullptr;
		m_pDevice = nullptr;
		m_pContext = nullptr;

		m_pCubeVertices = nullptr;
		m_pCubeIndices = nullptr;
		m_pSkyBoxDSS = nullptr;
		m_pSkyBoxRS = nullptr;

		m_pQuadVB = nullptr;
		m_pQuadIB = nullptr;
		m_pBoolBuffer = nullptr;
		m_pColorBuffer = nullptr;
		m_pSSAOKernelBuffer = nullptr;
		m_pExposureBuffer = nullptr;

		m_pScreenRTTV = nullptr;
		m_pDSBuffer = nullptr;
		m_pDSBufferForRTT = nullptr;
		m_pOutlinerSetupDSS = nullptr;
		m_pOutlinerTestDSS = nullptr;
		m_pDrawReflectionDSS = nullptr;
		m_pSSAORTT = nullptr;
		m_pSSAORTTV = nullptr;
		m_pSSAOSRV = nullptr;
		m_pSSAOBlurRTT = nullptr;
		m_pSSAOBlurRTTV = nullptr;
		m_pSSAOBlurSRV = nullptr;
		m_pViewportRTT = nullptr;
		m_pViewportRTTV = nullptr;
		m_pViewportSRV = nullptr;
		m_pSceneRTT = nullptr;
		m_pSceneRTTV = nullptr;
		m_pSceneSRV = nullptr;
		m_pBrightRTT = nullptr;
		m_pBrightRTTV = nullptr;
		m_pBrightSRV = nullptr;
		m_pSkyBoxSRV = nullptr;
		m_pScreenDSV = nullptr;
		m_pSceneDSV = nullptr;
		for (int i = 0; i < 2; ++i)
		{
			m_pPingPongRTT[i]  = nullptr;
			m_pPingPongRTTV[i] = nullptr;
			m_pPingPongSRV[i] =  nullptr;
		}
		for (int i = 0; i < _GBUF_CNT; ++i)
		{
			m_pGbufferRTT[i] = nullptr;
			m_pGbufferRTTV[i] = nullptr;
			m_pGbufferSRV[i] = nullptr;
		}
		m_pNoiseRTT = nullptr;
		m_pNoiseSRV = nullptr;

		m_pQuadRS = nullptr;
		m_pGeoRS = nullptr;
		m_pRasterStateCC = nullptr;

		m_pSampleState = nullptr;
		m_pSSAOPosSS = nullptr;

		m_pGBufferWriteBS = nullptr;
		m_pLightingPassBS = nullptr;

		m_pTerrain = nullptr;
		m_pImporter = nullptr;
		m_pCam = nullptr;
		m_pFrustum = nullptr;
		m_pLightBuffer = nullptr;
		m_pMatBuffer = nullptr;
		m_pShader = nullptr;
		m_pShadowMap = nullptr;

		m_selectedModelGroup = -1;
		m_selectedModel = -1;
		m_rotationVecs.reserve(16);
	}

	bool D3D11::Init(int screenWidth, int screenHeight, bool bVsync, HWND hWnd, bool bFullscreen,
		float fScreenFar, float fScreenNear)
	{
		HRESULT hr;
		bool result;
		IDXGIFactory* pFactory;
		IDXGIAdapter* pAdapter;
		IDXGIOutput* pAdapterOutput;
		unsigned int numModes, i, numerator, denominator;
		size_t strLen;
		DXGI_MODE_DESC* pDisplayModeList;
		DXGI_ADAPTER_DESC adapterDesc;
		int iError;
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		D3D_FEATURE_LEVEL featureLevel;
		ID3D11Texture2D* pBackbuffer;
		D3D11_RASTERIZER_DESC rasterDesc;
		D3D11_RENDER_TARGET_BLEND_DESC rtBlendDSC;
		float fFOV, fScreenAspect;
		m_bVsyncOn = bVsync;

		hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pFactory));
		if (FAILED(hr))
		{
			return false;
		}

		//Enumerates video cards
		hr = pFactory->EnumAdapters(0, &pAdapter);
		if (FAILED(hr))
		{
			return false;
		}

		//Enumerates outputs(ex:monitor)
		hr = pAdapter->EnumOutputs(0, &pAdapterOutput);
		if (FAILED(hr))
		{
			return false;
		}

		//Each monitor has a set of display modes it supports. A display mode refers to the following data in DXGI_MODE_DESC
		hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
		if (FAILED(hr))
		{
			return false;
		}

		//Fixing a display mode format, we can get a list of all supported display modes an output supports in that format with the following code :
		pDisplayModeList = new DXGI_MODE_DESC[numModes];
		if (pDisplayModeList == nullptr)
		{
			return false;
		}

		hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, pDisplayModeList);
		if (FAILED(hr))
		{
			return false;
		}

		hr = pAdapter->GetDesc(&adapterDesc);
		if (FAILED(hr))
		{
			return false;
		}


		delete[] pDisplayModeList;
		pDisplayModeList = nullptr;

		pAdapterOutput->Release();
		pAdapterOutput = nullptr;

		pAdapter->Release();
		pAdapter = nullptr;

		pFactory->Release();
		pFactory = nullptr;

		m_clientWidth = screenWidth;
		m_clientHeight = screenHeight;

		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = screenWidth;
		swapChainDesc.BufferDesc.Height = screenHeight;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;


		if (m_bVsyncOn == true)
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 75;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		}
		else
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		}

		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = hWnd;

		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		if (bFullscreen == true)
		{
			swapChainDesc.Windowed = FALSE;
		}
		else
		{
			swapChainDesc.Windowed = TRUE;
		}

		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		featureLevel = D3D_FEATURE_LEVEL_11_0;


		hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1,
			D3D11_SDK_VERSION, &swapChainDesc, &m_pSwapChain, &m_pDevice, nullptr, &m_pContext);
		if (FAILED(hr))
		{
			return false;
		}

		hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackbuffer));
		if (FAILED(hr))
		{
			return false;
		}

		hr = m_pDevice->CreateRenderTargetView(pBackbuffer, nullptr, &m_pScreenRTTV);
		if (FAILED(hr))
		{
			return false;
		}

		pBackbuffer->Release();
		pBackbuffer = nullptr;
		{
			XMFLOAT3 vertices[] = {
				{XMFLOAT3(-1.0f, -1.0f, 0.0f)},
				{XMFLOAT3(-1.0f, 1.0f, 0.0f)},
				{XMFLOAT3(1.0f, 1.0f, 0.0f)},
				{XMFLOAT3(1.0f, -1.0f, 0.0f)}
			};
			XMFLOAT2 texCoords[] = {
				{XMFLOAT2(0.0f, 1.0f)},
				{XMFLOAT2(0.0f, 0.0f)},
				{XMFLOAT2(1.0f, 0.0f)},
				{XMFLOAT2(1.0f, 1.0f)}
			};

			QUAD quads[4];
			for (int i = 0; i < 4; ++i)
			{
				quads[i].pos = vertices[i];
				quads[i].tex = texCoords[i];
			}

			D3D11_SUBRESOURCE_DATA quadData;
			quadData.pSysMem = quads;
			quadData.SysMemPitch = 0;
			quadData.SysMemSlicePitch = 0;

			D3D11_BUFFER_DESC quadBDSC = {0,};
			quadBDSC.Usage = D3D11_USAGE_DEFAULT;
			quadBDSC.ByteWidth = sizeof(QUAD) * 4;
			quadBDSC.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		

			hr = m_pDevice->CreateBuffer(&quadBDSC, &quadData, &m_pQuadVB);
			if (FAILED(hr))
			{
				return false;
			}

			unsigned long indices[6] = { 0,1,2, 2,3,0 };
			D3D11_SUBRESOURCE_DATA indexData;
			indexData.pSysMem = indices;
			indexData.SysMemPitch = 0;
			indexData.SysMemSlicePitch = 0;

			D3D11_BUFFER_DESC indexBD = { 0, };
			indexBD.Usage = D3D11_USAGE_DEFAULT;
			indexBD.ByteWidth = sizeof(unsigned long) * 6;
			indexBD.BindFlags = D3D11_BIND_INDEX_BUFFER;

			hr = m_pDevice->CreateBuffer(&indexBD, &indexData, &m_pQuadIB);
			if (FAILED(hr))
			{
				return false;
			}
		}
		{	
			XMFLOAT3 vertices[] = {
			 {XMFLOAT3(-1.0f,  1.0f, -1.0f)},//front-upper-left  0
			 {XMFLOAT3(1.0f,   1.0f, -1.0f)},//front-upper-right 1
			 {XMFLOAT3(1.0f,  -1.0f, -1.0f)},//front-down-right  2
			 {XMFLOAT3(-1.0f, -1.0f, -1.0f)},//front-down-left   3

			 {XMFLOAT3(-1.0f,  1.0f, 1.0f)},//back-upper-left   4
			 {XMFLOAT3(1.0f,   1.0f, 1.0f)},//back-upper-right  5
			 {XMFLOAT3(1.0f,  -1.0f, 1.0f)},//back-down-right   6
			 {XMFLOAT3(-1.0f, -1.0f, 1.0f)}};//back-down-left   7
			int vertexCount = sizeof(vertices) / sizeof(XMFLOAT3);
			
			 D3D11_SUBRESOURCE_DATA skyBoxVertexData;
			 skyBoxVertexData.pSysMem = vertices;
			 skyBoxVertexData.SysMemPitch = 0;
			 skyBoxVertexData.SysMemSlicePitch = 0;

			D3D11_BUFFER_DESC skyBoxVertexBD;
			skyBoxVertexBD.Usage = D3D11_USAGE_DEFAULT;
			skyBoxVertexBD.ByteWidth = sizeof(XMFLOAT3) * vertexCount;
			skyBoxVertexBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			skyBoxVertexBD.CPUAccessFlags = 0;
			skyBoxVertexBD.MiscFlags = 0;
			skyBoxVertexBD.StructureByteStride = 0;

		    hr = m_pDevice->CreateBuffer(&skyBoxVertexBD,&skyBoxVertexData, &m_pCubeVertices);
			if (FAILED(hr))
			{
				return false;
			}
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
			int indexCount = sizeof(indices) / sizeof(unsigned long);

			D3D11_SUBRESOURCE_DATA skyBoxIndexData;
			skyBoxIndexData.pSysMem = indices;
			skyBoxIndexData.SysMemPitch = 0;
			skyBoxIndexData.SysMemSlicePitch = 0;

			D3D11_BUFFER_DESC skyBoxIndexBD;
			skyBoxIndexBD.Usage = D3D11_USAGE_DEFAULT;
			skyBoxIndexBD.ByteWidth = sizeof(unsigned long) * indexCount;
			skyBoxIndexBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
			skyBoxIndexBD.CPUAccessFlags = 0;
			skyBoxIndexBD.MiscFlags = 0;
			skyBoxIndexBD.StructureByteStride = 0;

			hr = m_pDevice->CreateBuffer(&skyBoxIndexBD, &skyBoxIndexData, &m_pCubeIndices);
			if(FAILED(hr))
			{
				return false;
			}
			D3DX11_IMAGE_LOAD_INFO loadInfo = {};
			loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			loadInfo.Filter = D3DX11_FILTER_SRGB | D3DX11_FILTER_NONE;
			
			hr = D3DX11CreateShaderResourceViewFromFileW(m_pDevice, L"./Assets/Textures/ocean.dds",
				&loadInfo, 0, &m_pSkyBoxSRV, 0);
			if (FAILED(hr))
			{
				return false;
			}

			D3D11_DEPTH_STENCIL_DESC skyboxDSD;
			ZeroMemory(&skyboxDSD, sizeof(D3D11_DEPTH_STENCIL_DESC));
			skyboxDSD.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			hr = m_pDevice->CreateDepthStencilState(&skyboxDSD, &m_pSkyBoxDSS);
			if (FAILED(hr))
			{
				return false;
			}

			D3D11_RASTERIZER_DESC skyboxRD;
			ZeroMemory(&skyboxRD, sizeof(D3D11_RASTERIZER_DESC));
			skyboxRD.FillMode = D3D11_FILL_SOLID;
			skyboxRD.CullMode = D3D11_CULL_BACK;
			hr = m_pDevice->CreateRasterizerState(&skyboxRD, &m_pSkyBoxRS);
			if (FAILED(hr))
			{
				return false;
			}

		}

		if (!CreateRTT(m_clientWidth, m_clientHeight))
		{
			return false;
		}

		if (!CreateDSS())
		{
			return false;
		}

		if (!CreateDepthBuffer(m_clientWidth, m_clientHeight, &m_pDSBuffer, &m_pScreenDSV))
		{
			return false;
		}
		
		if (!CreateDepthBuffer(m_clientWidth, m_clientHeight, &m_pDSBufferForRTT, &m_pSceneDSV))
		{
			return false;
		}

		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.DepthBias = 0.0f;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = true;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 1.0f;

		hr = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pGeoRS);
		if (FAILED(hr))
		{
			return false;
		}

		rasterDesc.FrontCounterClockwise = false;
		hr = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pQuadRS);
		if (FAILED(hr))
		{
			return false;
		}
		
		m_viewport.Width = static_cast<float>(screenWidth);
		m_viewport.Height = static_cast<float>(screenHeight);
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		hr = m_pDevice->CreateSamplerState(&samplerDesc, &m_pSampleState);
		if (FAILED(hr))
		{
			return false;
		}

		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		hr = m_pDevice->CreateSamplerState(&samplerDesc, &m_pSSAOPosSS);
		if (FAILED(hr))
		{
			return false;
		}


		rtBlendDSC.BlendEnable = TRUE;
		rtBlendDSC.SrcBlend = D3D11_BLEND_ONE;
		rtBlendDSC.DestBlend = D3D11_BLEND_ZERO;
		rtBlendDSC.BlendOp = D3D11_BLEND_OP_ADD;
		rtBlendDSC.SrcBlendAlpha = D3D11_BLEND_ONE;
		rtBlendDSC.DestBlendAlpha = D3D11_BLEND_ZERO;
		rtBlendDSC.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtBlendDSC.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		D3D11_BLEND_DESC blendDSC = { false, false, rtBlendDSC };
		hr = m_pDevice->CreateBlendState(&blendDSC, &m_pGBufferWriteBS);
		if (FAILED(hr))
		{
			return false;
		}

		rtBlendDSC.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDSC.RenderTarget[0] = rtBlendDSC;
		hr = m_pDevice->CreateBlendState(&blendDSC, &m_pLightingPassBS);
		if (FAILED(hr))
		{
			return false;
		}
		
		//Set projectionMatrix, viewMatrix;
		m_pTerrain = new Terrain;
		m_pTerrain->Init(m_pDevice, 100, 100);
		m_pCam = new Camera(screenWidth, screenHeight, fScreenFar, fScreenNear);
		m_pCam->Init(m_pDevice);
		m_pCam->SetCamPos(m_pContext);
		XMMATRIX* m_projMat = m_pCam->GetProjectionMatrix();
		XMMATRIX* m_viewMat = m_pCam->GetViewMatrix();
		m_pFrustum = new Frustum();
		m_pFrustum->Construct(100.0f, m_pCam);
		m_pMatBuffer = new MatBuffer(m_pDevice, m_pContext, m_viewMat, m_projMat);
		m_pMatBuffer->Init();

		m_pLightBuffer = new LightBuffer(m_pDevice);

		m_pShader = new Shader(m_pDevice, m_pContext);
		m_pShader->Init();

		m_pShadowMap = new ShadowMap();
		m_pShadowMap->Init(m_pDevice, SHADOWMAP_SIZE, SHADOWMAP_SIZE,
			m_pLightBuffer->GetDirLightCapacity(), m_pLightBuffer->GetPointLightCapacity(), m_pLightBuffer->GetSpotLightCapacity());
		{
			D3D11_BUFFER_DESC bds = { 0, };
			bds.ByteWidth = sizeof(BOOL) * 4;
			bds.Usage = D3D11_USAGE_DYNAMIC;
			bds.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bds.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			hr = m_pDevice->CreateBuffer(&bds, 0, &m_pBoolBuffer);
			if (FAILED(hr))
			{
				return false;
			}

			bds.ByteWidth = sizeof(XMVECTOR);
			hr = m_pDevice->CreateBuffer(&bds, 0, &m_pColorBuffer);
			if (FAILED(hr))
			{
				return false;
			}
			
			bds.ByteWidth = sizeof(SamplePoints);
			hr = m_pDevice->CreateBuffer(&bds, 0, &m_pSSAOKernelBuffer);
			if (FAILED(hr))
			{
				return false;
			}

			bds.ByteWidth = sizeof(XMFLOAT4);
			hr = m_pDevice->CreateBuffer(&bds, 0, &m_pExposureBuffer);
			if (FAILED(hr))
			{
				return false;
			}
		}
		{	

			//Gen Sample points
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			SamplePoints* pSamplePoints;
			hr = m_pContext->Map(m_pSSAOKernelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(hr))
			{
				return false;
			}
			pSamplePoints = (SamplePoints*)mappedResource.pData;

			std::uniform_real_distribution<float> randomFloats(0.0, 1.0f);
			std::default_random_engine gen;
			for (int i = 0; i < 64; ++i)
			{	
				XMFLOAT3 sample(
					randomFloats(gen) * 2.0f - 1.0f,
					randomFloats(gen) * 2.0f - 1.0f,
					randomFloats(gen));
				XMVECTOR sampleV = XMLoadFloat3(&sample);
				sampleV = DirectX::XMVector3Normalize(sampleV);
				sampleV = XMVectorScale(sampleV, randomFloats(gen));
				float scale = i / 64.0f;
				scale = 0.1f + (1.0f - 0.1f) * scale * scale;
				sampleV = XMVectorScale(sampleV, scale);

				pSamplePoints->coord[i] = sampleV;
			}
			m_pContext->Unmap(m_pSSAOKernelBuffer, 0);


			//Gen noise texture
			for (int i = 0; i < 16; ++i)
			{
				XMFLOAT3 rot(
					randomFloats(gen) * 2.0f - 1.0f,
					randomFloats(gen) * 2.0f - 1.0f,
					0.0f);
				m_rotationVecs.push_back(rot);
			}
			D3D11_TEXTURE2D_DESC texDesc = {};
			texDesc.Width = 4;
			texDesc.Height = 4;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			texDesc.ArraySize = 1;
			texDesc.MipLevels = 1;
			texDesc.SampleDesc.Count = 1;
			D3D11_SUBRESOURCE_DATA data = { 0, };
			data.pSysMem = &m_rotationVecs[0];
			data.SysMemPitch = texDesc.Width * sizeof(XMFLOAT3);

			hr = m_pDevice->CreateTexture2D(&texDesc, &data, &m_pNoiseRTT);
			if (FAILED(hr))
			{
				return false;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = texDesc.Format;
			srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			hr = m_pDevice->CreateShaderResourceView(m_pNoiseRTT, &srvDesc, &m_pNoiseSRV);
			if (FAILED(hr))
			{
				return false;
			}

		}

		ImGui_ImplDX11_Init(m_pDevice, m_pContext);
		return true;
	}
	void D3D11::Shutdown()
	{
		ImGui_ImplDX11_Shutdown();
		if (m_pQuadVB != nullptr)
		{
			m_pQuadVB->Release();
			m_pQuadVB = nullptr;
		}
		if (m_pQuadIB != nullptr)
		{
			m_pQuadIB->Release();
			m_pQuadIB = nullptr;
		}

		if (m_pQuadRS != nullptr)
		{
			m_pQuadRS->Release();
			m_pQuadRS = nullptr;
		}

		if (m_pCubeVertices != nullptr)
		{
			m_pCubeVertices->Release();
			m_pCubeVertices = nullptr;
		}

		if (m_pCubeIndices != nullptr)
		{
			m_pCubeIndices->Release();
			m_pCubeIndices = nullptr;
		}

		if (m_pSkyBoxDSS != nullptr)
		{
			m_pSkyBoxDSS->Release();
			m_pSkyBoxDSS = nullptr;
		}

		if (m_pSkyBoxRS != nullptr)
		{
			m_pSkyBoxRS->Release();
			m_pSkyBoxRS = nullptr;
		}

		if (m_pGeoRS != nullptr)
		{
			m_pGeoRS->Release();
			m_pGeoRS = nullptr;
		}

		if (m_pRasterStateCC != nullptr)
		{
			m_pRasterStateCC->Release();
			m_pRasterStateCC = nullptr;
		}

		DestroyRTT();
		DestroyDSBforRTT();
		if (m_pDSBuffer != nullptr)
		{
			m_pDSBuffer->Release();
			m_pDSBuffer = nullptr;
		}

		if (m_pScreenDSV != nullptr)
		{
			m_pScreenDSV->Release();
			m_pScreenDSV = nullptr;
		}

		DestroyDSS();
		if (m_pSkyBoxSRV != nullptr)
		{
			m_pSkyBoxSRV->Release();
			m_pSkyBoxSRV = nullptr;
		}

		if (m_pScreenRTTV != nullptr)
		{
			m_pScreenRTTV->Release();
			m_pScreenRTTV = nullptr;
		}

		if (m_pNoiseRTT != nullptr)
		{
			m_pNoiseRTT->Release();
			m_pNoiseRTT = nullptr;
		}

		if (m_pNoiseSRV != nullptr)
		{
			m_pNoiseSRV->Release();
			m_pNoiseSRV = nullptr;
		}

		if (m_pContext != nullptr)
		{
			m_pContext->Release();
			m_pContext = nullptr;
		}

		if (m_pDevice != nullptr)
		{
			m_pDevice->Release();
			m_pDevice = nullptr;
		}

		if (m_pSwapChain != nullptr)
		{	
			m_pSwapChain->Release();
			m_pSwapChain = nullptr;
		}

		if (m_pSampleState != nullptr)
		{
			m_pSampleState->Release();
			m_pSampleState = nullptr;
		}

		if (m_pSSAOPosSS != nullptr)
		{
			m_pSSAOPosSS->Release();
			m_pSSAOPosSS = nullptr;
		}

		if (m_pImporter != nullptr)
		{
			delete m_pImporter;
			m_pImporter = nullptr;
		}

		if (m_pBoolBuffer != nullptr)
		{
			m_pBoolBuffer->Release();
			m_pBoolBuffer = nullptr;
		}

		if (m_pColorBuffer != nullptr)
		{
			m_pColorBuffer->Release();
			m_pColorBuffer = nullptr;
		}

		if (m_pSSAOKernelBuffer != nullptr)
		{
			m_pSSAOKernelBuffer->Release();
			m_pSSAOKernelBuffer = nullptr;
		}

		if (m_pExposureBuffer != nullptr)
		{
			m_pExposureBuffer->Release();
			m_pExposureBuffer = nullptr;
		}

		if (m_pLightBuffer != nullptr)
		{
			delete m_pLightBuffer;
			m_pLightBuffer = nullptr;
		}

		if (m_pMatBuffer != nullptr)
		{
			delete m_pMatBuffer;
			m_pMatBuffer = nullptr;
		}

		if (m_pTerrain != nullptr)
		{
			delete m_pTerrain;
			m_pTerrain = nullptr;
		}

		if (m_pCam != nullptr)
		{
			delete m_pCam;
			m_pCam = nullptr;
		}

		if (m_pFrustum != nullptr)
		{
			delete m_pFrustum;
			m_pFrustum = nullptr;
		}

		if (m_pShader != nullptr)
		{
			delete m_pShader;
			m_pShader = nullptr;
		}

		if (m_pShadowMap != nullptr)
		{
			delete m_pShadowMap;
			m_pShadowMap = nullptr;
		}

		if (m_pGBufferWriteBS != nullptr)
		{
			m_pGBufferWriteBS->Release();
			m_pGBufferWriteBS = nullptr;
		}

		if (m_pLightingPassBS != nullptr)
		{
			m_pLightingPassBS->Release();
			m_pLightingPassBS = nullptr;
		}

		for (int i = 0; i < m_pModelGroups.size(); ++i)
		{
			if (m_pModelGroups[i] != nullptr)
			{
				m_pModelGroups[i]->Clear();
				delete m_pModelGroups[i];
			}
		}
		m_pModelGroups.clear();
		m_pModelGroups.shrink_to_fit();
		return;
	}

	void D3D11::UpdateScene()
	{
		//clear views
		HRESULT hr;
		float color[4] = { 0.0f, 0.0f,0.0f, 1.0f };
		UINT stride;
		UINT offset = 0;
		int drawed = 0;
		bool bGeoPass = false;
		
		ID3D11RenderTargetView* nullRTV[_GBUF_CNT] = { nullptr, };

		m_pContext->ClearRenderTargetView(m_pScreenRTTV, color);
		m_pContext->ClearRenderTargetView(m_pViewportRTTV, color);
		m_pContext->ClearRenderTargetView(m_pSceneRTTV, color);
		for (int i = 0; i < _GBUF_CNT; ++i)
		{
			m_pContext->ClearRenderTargetView(m_pGbufferRTTV[i], color);
		}

		m_pContext->ClearDepthStencilView(m_pScreenDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_pContext->ClearDepthStencilView(m_pSceneDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pContext->PSSetSamplers(0, 1, &m_pSampleState);
		//Update Cam 
		m_pCam->Update();
		//Update Light
		std::vector<DirectionalLight*>& dirLights = m_pLightBuffer->GetDirLights();
		std::vector<PointLight*>& pointLights = m_pLightBuffer->GetPointLights();
		std::vector<SpotLight*>& spotLights = m_pLightBuffer->GetSpotLights();
		//Draw ShadowMap
		{	
			m_pContext->PSSetShaderResources(0, 4 +dirLights.capacity()+ pointLights.capacity()+ spotLights.capacity(), m_pLightBuffer->GetNullSRVs());
			m_pContext->RSSetViewports(1, m_pShadowMap->GetViewport());
			m_pContext->RSSetState(m_pGeoRS);
			m_pContext->OMSetDepthStencilState(0, 0);
			
			m_pShader->SetTexInputlayout();
			m_pShader->SetShadowShader();
			
			for (int i = 0; i < dirLights.size(); ++i)
			{	
				m_pMatBuffer->SetLightSpaceMatrix(dirLights[i]->GetLightSpaceMat());
				m_pShadowMap->BindDirDSV(m_pContext,i);
				DrawENTT(bGeoPass);
			}
			for (int i = 0; i < spotLights.size(); ++i)
			{
				m_pMatBuffer->SetLightSpaceMatrix(spotLights[i]->GetLightSpaceMat());
				m_pShadowMap->BindSpotDSV(m_pContext, i);
				DrawENTT(bGeoPass);
			}


			m_pContext->RSSetState(m_pSkyBoxRS);
			m_pShader->SetOmniDirShadowShader();
			for (int i = 0; i < pointLights.size(); ++i)
			{
				m_pShadowMap->BindCubeDSV(m_pContext, i);
				pointLights[i]->SetShadowMatrices(m_pContext);
				pointLights[i]->SetLightPos(m_pContext);
				DrawENTT(bGeoPass);
			}
			
		}
		//Draw EnvMap
		stride = sizeof(XMFLOAT3);
		m_pMatBuffer->SetWorldMatrix(&m_idMat);	
		m_pMatBuffer->SetViewMatrix(m_pCam->GetViewMatrix());
		m_pMatBuffer->SetProjMatrix(m_pCam->GetProjectionMatrix());
		m_pMatBuffer->Update();
		m_pShader->SetSkyBoxShader();
		m_pContext->IASetVertexBuffers(0, 1, &m_pCubeVertices, &stride, &offset);
		m_pContext->IASetIndexBuffer(m_pCubeIndices, DXGI_FORMAT_R32_UINT, 0);
		m_pContext->PSSetShaderResources(0, 1, &m_pSkyBoxSRV);
		m_pContext->RSSetViewports(1, &m_viewport);
		m_pContext->OMSetDepthStencilState(m_pSkyBoxDSS, 0);
		m_pContext->OMSetRenderTargets(1, &m_pGbufferRTTV[2], m_pSceneDSV);
		m_pContext->DrawIndexed(36, 0, 0);
	
		//Deferred Shading First Pass
		if (!m_pModelGroups.empty())
		{
			m_pShader->SetDeferredGeoLayout();
			m_pShader->SetPBRDeferredGeoShader();
			m_pContext->OMSetDepthStencilState(0, 0);
			m_pContext->RSSetState(m_pQuadRS);
			m_pContext->OMSetBlendState(m_pGBufferWriteBS, color, 0xffffffff);
			m_pContext->OMSetRenderTargets(_GBUF_CNT, m_pGbufferRTTV, m_pSceneDSV);
			DrawENTT(!bGeoPass);

			// PBR FBX들은 대부분 AO 맵이 달려온다.
			m_pShader->SetTexInputlayout();
			m_pShader->SetPBRDeferredLightingShader();
			m_pLightBuffer->UpdateDirLightMatrices(m_pContext);
			m_pLightBuffer->UpdateSpotLightMatrices(m_pContext);
			m_pLightBuffer->UpdateLightBuffer(m_pContext);
			m_pCam->SetCamPos(m_pContext);
			stride = sizeof(QUAD);
			m_pContext->IASetVertexBuffers(0, 1, &m_pQuadVB, &stride, &offset);
			m_pContext->IASetIndexBuffer(m_pQuadIB, DXGI_FORMAT_R32_UINT, 0);
			m_pContext->RSSetState(m_pQuadRS);
			m_pContext->OMSetRenderTargets(1, &m_pSceneRTTV, nullptr);
			m_pContext->OMSetBlendState(m_pLightingPassBS, color, 0xffffffff);
			m_pContext->PSSetShaderResources(0, _GBUF_CNT, m_pGbufferSRV);
			m_pContext->PSSetShaderResources(_GBUF_CNT,  dirLights.capacity(), m_pShadowMap->GetDirSRV());
			m_pContext->PSSetShaderResources(_GBUF_CNT + dirLights.capacity(), spotLights.capacity(), m_pShadowMap->GetSpotSRV());
			m_pContext->PSSetShaderResources(_GBUF_CNT + dirLights.capacity() + spotLights.capacity(), pointLights.capacity(), m_pShadowMap->GetCubeSRV());
			m_pContext->PSSetSamplers(1, 1, m_pShadowMap->GetCubeShadowSampler());
			m_pContext->PSSetSamplers(2, 1, m_pShadowMap->GetDirShadowSampler());
			m_pContext->DrawIndexed(6, 0, 0);
			
		}
		
		//Draw picked Model's Outline
		m_pShader->SetTexInputlayout();
		m_pShader->SetOutlinerShader();
		m_pContext->OMSetDepthStencilState(m_pOutlinerTestDSS, 1);
		if (m_selectedModelGroup != -1)
		{
			std::vector<Model*> pModels = m_pModelGroups[m_selectedModelGroup]->GetModels();
			XMMATRIX worldMat = pModels[m_selectedModel]->GetTransformMatrix(true);
			m_pMatBuffer->SetWorldMatrix(&worldMat);
			m_pMatBuffer->Update();

			for (int k = 0; k < pModels[m_selectedModel]->GetMatCount(); ++k)
			{
				pModels[m_selectedModel]->UploadBuffers(m_pContext, k, bGeoPass);
				m_pContext->DrawIndexed(pModels[m_selectedModel]->GetIndexCount(k), 0, 0);
			}
		}
		//Submit Result
		stride = sizeof(QUAD);
		m_pContext->IASetVertexBuffers(0, 1, &m_pQuadVB, &stride, &offset);
		m_pContext->IASetIndexBuffer(m_pQuadIB, DXGI_FORMAT_R32_UINT, 0);
		m_pShader->SetTexInputlayout();
		m_pShader->SetFinalShader();
		m_pContext->RSSetState(m_pQuadRS);
		m_pContext->OMSetRenderTargets(0, nullptr, nullptr);
		{	
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			XMFLOAT3* pExposure;
			m_pContext->Map(m_pExposureBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			pExposure = (XMFLOAT3*)mappedResource.pData;
			pExposure->x = m_exposure;
			m_pContext->Unmap(m_pExposureBuffer, 0);
			m_pContext->PSSetConstantBuffers(0, 1, &m_pExposureBuffer);
		}
		
		m_pContext->PSSetShaderResources(0, 1, m_pModelGroups.empty() ? &m_pGbufferSRV[2] : &m_pSceneSRV);
		m_pContext->OMSetRenderTargets(1, &m_pViewportRTTV, nullptr);
		m_pContext->OMSetDepthStencilState(0, 0);
		m_pContext->DrawIndexed(6, 0, 0);
		//DrawUI
		m_pContext->OMSetRenderTargets(1, &m_pScreenRTTV, m_pScreenDSV);
		return;
	}
	void D3D11::DrawScene()
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}


		if (m_bVsyncOn == true)
		{
			m_pSwapChain->Present(1, 0);
		}
		else
		{
			m_pSwapChain->Present(0, 0);
		}
	}

	void D3D11::AddLight(Light* pLight)
	{	
		pLight->Init(m_pDevice);
		switch (pLight->GetType())
		{
		case ELIGHT_TYPE::DIR:
			m_pLightBuffer->PushDirLight((DirectionalLight*)pLight);
			break;
		case ELIGHT_TYPE::PNT:
			m_pLightBuffer->PushPointLight((PointLight*)pLight);
			break;
		case ELIGHT_TYPE::SPT:
			m_pLightBuffer->PushSpotLight((SpotLight*)pLight);
		}
	}
	void D3D11::AddModelGroup(ModelGroup* pModelGroup, ID3D11Device* pDevice)
	{
		m_pModelGroups.push_back(pModelGroup);
		m_pModelGroups.back()->Init(pDevice);
	}
	void D3D11::RemoveModelGroup(int i)
	{
		delete m_pModelGroups[i];
		m_pModelGroups.erase(m_pModelGroups.begin() + i);
	}
	void D3D11::RemoveLight(int i, Light* pLight)
	{	
		std::vector<DirectionalLight*>& pDirLights = m_pLightBuffer->GetDirLights();;
		std::vector<PointLight*>& pPointLights= m_pLightBuffer->GetPointLights();
		std::vector<SpotLight*>& pSpotLights = m_pLightBuffer->GetSpotLights();
		switch (pLight->GetType())
		{
		case ELIGHT_TYPE::DIR:
			delete pDirLights[i];
			pDirLights.erase(pDirLights.begin() + i);
			break;
		case ELIGHT_TYPE::PNT:
			delete pPointLights[i];
			pPointLights.erase(pPointLights.begin() + i);
			break;
		case ELIGHT_TYPE::SPT:
			delete pSpotLights[i];
			pSpotLights.erase(pSpotLights.begin() + i);
			break;
		}
	}
	void D3D11::RemoveModel(int i, int j)
	{
		std::vector<Model*>& pModels = m_pModelGroups[i]->GetModels();
		delete pModels[j];
		pModels.erase(pModels.begin() + j);
	}
	UINT D3D11::GetLightSize(Light* pLight)
	{
		UINT size=0;
		switch (pLight->GetType())
		{
		case ELIGHT_TYPE::DIR:
			size = m_pLightBuffer->GetDirLightSize();
			break;
		case ELIGHT_TYPE::PNT:
			size = m_pLightBuffer->GetPointLightSize();
			break;
		case ELIGHT_TYPE::SPT:
			size = m_pLightBuffer->GetSpotLightSize();
		}
		return size;
	}

	bool D3D11::CreateRTT(int width, int height)
	{
		DestroyRTT();

		D3D11_TEXTURE2D_DESC RTTDesc;
		D3D11_RENDER_TARGET_VIEW_DESC RTTVDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		HRESULT hr;

		ZeroMemory(&RTTDesc, sizeof(RTTDesc));
		RTTDesc.Width = width;;
		RTTDesc.Height = height;
		RTTDesc.MipLevels = 1;
		RTTDesc.ArraySize = 1;
		RTTDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		RTTDesc.SampleDesc.Count = 1;
		RTTDesc.SampleDesc.Quality = 0;
		RTTDesc.Usage = D3D11_USAGE_DEFAULT;
		RTTDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		RTTDesc.CPUAccessFlags = 0;
		RTTDesc.MiscFlags = 0;

		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pViewportRTT);
		if (FAILED(hr))
		{
			return false;
		}
		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pSceneRTT);
		if (FAILED(hr))
		{
			return false;
		}
		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pBrightRTT);
		if (FAILED(hr))
		{
			return false;
		}
		for (int i = 0; i < 2; ++i)
		{
			hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pPingPongRTT[i]);
			if (FAILED(hr))
			{
				return false;
			}
		}
		for (int i = 0; i < _GBUF_CNT; ++i)
		{
			hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pGbufferRTT[i]);
			if (FAILED(hr))
			{
				return false;
			}
		}


		RTTVDesc.Format = RTTDesc.Format;
		RTTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		RTTVDesc.Texture2D.MipSlice = 0;

		hr = m_pDevice->CreateRenderTargetView(m_pViewportRTT, &RTTVDesc, &m_pViewportRTTV);
		if (FAILED(hr))
		{
			return false;
		}
		hr = m_pDevice->CreateRenderTargetView(m_pSceneRTT, &RTTVDesc, &m_pSceneRTTV);
		if (FAILED(hr))
		{
			return false;
		}
		hr = m_pDevice->CreateRenderTargetView(m_pBrightRTT, &RTTVDesc, &m_pBrightRTTV);
		if (FAILED(hr))
		{
			return false;
		}
		for (int i = 0; i < 2; ++i)
		{
			hr = m_pDevice->CreateRenderTargetView(m_pPingPongRTT[i], &RTTVDesc, &m_pPingPongRTTV[i]);
			if (FAILED(hr))
			{
				return false;
			}
		}
		for (int i = 0; i < _GBUF_CNT; ++i)
		{
			hr = m_pDevice->CreateRenderTargetView(m_pGbufferRTT[i], &RTTVDesc, &m_pGbufferRTTV[i]);
			if (FAILED(hr))
			{
				return false;
			}
		}


		SRVDesc.Format = RTTDesc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = m_pDevice->CreateShaderResourceView(m_pViewportRTT, &SRVDesc, &m_pViewportSRV);
		if (FAILED(hr))
		{
			return false;
		}
		hr = m_pDevice->CreateShaderResourceView(m_pSceneRTT, &SRVDesc, &m_pSceneSRV);
		if (FAILED(hr))
		{
			return false;
		}
		hr = m_pDevice->CreateShaderResourceView(m_pBrightRTT, &SRVDesc, &m_pBrightSRV);
		if (FAILED(hr))
		{
			return false;
		}
		for (int i = 0; i < 2; ++i)
		{
			hr = m_pDevice->CreateShaderResourceView(m_pPingPongRTT[i], &SRVDesc, &m_pPingPongSRV[i]);
			if (FAILED(hr))
			{
				return false;
			}
		}
		for (int i = 0; i < _GBUF_CNT; ++i)
		{
			hr = m_pDevice->CreateShaderResourceView(m_pGbufferRTT[i], &SRVDesc, &m_pGbufferSRV[i]);
			if (FAILED(hr))
			{
				return false;
			}
		}

		RTTDesc.Format = DXGI_FORMAT_R32_FLOAT;
		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pSSAORTT);
		if (FAILED(hr))
		{
			return false;
		}
		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pSSAOBlurRTT);
		if (FAILED(hr))
		{
			return false;
		}
		RTTVDesc.Format = RTTDesc.Format;
		hr = m_pDevice->CreateRenderTargetView(m_pSSAORTT, &RTTVDesc, &m_pSSAORTTV);
		if (FAILED(hr))
		{
			return false;
		}
		hr = m_pDevice->CreateRenderTargetView(m_pSSAOBlurRTT, &RTTVDesc, &m_pSSAOBlurRTTV);
		if (FAILED(hr))
		{
			return false;
		}
		SRVDesc.Format = RTTVDesc.Format;
		hr = m_pDevice->CreateShaderResourceView(m_pSSAORTT, &SRVDesc, &m_pSSAOSRV);
		if (FAILED(hr))
		{
			return false;
		}
		hr = m_pDevice->CreateShaderResourceView(m_pSSAOBlurRTT, &SRVDesc, &m_pSSAOBlurSRV);
		if (FAILED(hr))
		{
			return false;
		}
		return true;
	}
	bool D3D11::CreateDepthBuffer(int width, int height,
		ID3D11Texture2D** dsb,
		ID3D11DepthStencilView** dsbv)
	{
		D3D11_TEXTURE2D_DESC depthBufferDesc;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		HRESULT hr;

		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		depthBufferDesc.Width = width;
		depthBufferDesc.Height = height;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;

		hr = m_pDevice->CreateTexture2D(&depthBufferDesc, nullptr, dsb);
		if (FAILED(hr))
		{
			return false;
		}

		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;


		hr = m_pDevice->CreateDepthStencilView(*dsb, &depthStencilViewDesc, dsbv);
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}
	bool D3D11::CreateDSS()
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		HRESULT hr;

		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

		//Need depthStencilState to control depthStencilBuffer
		//To create it, fill out depthStencilDesc
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthStencilDesc.StencilEnable = true;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		hr = m_pDevice->CreateDepthStencilState(&depthStencilDesc, &m_pOutlinerSetupDSS);
		if (FAILED(hr))
		{
			return false;
		}

		depthStencilDesc.DepthEnable = false;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;

		hr = m_pDevice->CreateDepthStencilState(&depthStencilDesc, &m_pOutlinerTestDSS);
		if (FAILED(hr))
		{
			return false;
		}

		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
		hr = m_pDevice->CreateDepthStencilState(&depthStencilDesc, &m_pDrawReflectionDSS);
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}

	void D3D11::DrawENTT(bool bGeoPass)
	{
		XMMATRIX worldMat;
		for (int i = 0; i < m_pModelGroups.size(); ++i)
		{	

			std::vector<Model*> pModels = m_pModelGroups[i]->GetModels();
			//if (m_pFrustum->IsInFrustum(XMVectorSet(pos4._41, pos4._42, pos4._43, pos4._44)))
			{
				
				{
					for (int j = 0; j < pModels.size(); ++j)
					{	
						bool isSelected = (i == m_selectedModelGroup && j == m_selectedModel);
						if (isSelected)
						{
							m_pContext->OMSetDepthStencilState(m_pOutlinerSetupDSS, 1);
						}
						worldMat = pModels[j]->GetTransformMatrix(false);
						m_pMatBuffer->SetWorldMatrix(&worldMat);
						m_pMatBuffer->Update();
						
						for (int k = 0; k < pModels[j]->GetMatCount(); ++k)
						{
							pModels[j]->UploadBuffers(m_pContext,k, bGeoPass);
							m_pContext->DrawIndexed(pModels[j]->GetIndexCount(k), 0, 0);
						}

						if (isSelected)
						{
							m_pContext->OMSetDepthStencilState(0, 0);
						}
						
					}

				}

				
			}

		}
	}

	void D3D11::DestroyDSS()
	{
		if (m_pOutlinerSetupDSS != nullptr)
		{
			m_pOutlinerSetupDSS->Release();
			m_pOutlinerSetupDSS = nullptr;
		}

		if (m_pOutlinerTestDSS != nullptr)
		{
			m_pOutlinerTestDSS->Release();
			m_pOutlinerTestDSS = nullptr;
		}

		if (m_pDrawReflectionDSS != nullptr)
		{
			m_pDrawReflectionDSS->Release();
			m_pDrawReflectionDSS = nullptr;
		}
	}
	void D3D11::DestroyRTT()
	{
		if (m_pSceneRTT != nullptr)
		{
			m_pSceneRTT->Release();
			m_pSceneRTT = nullptr;
		}

		if (m_pSceneRTTV != nullptr)
		{
			m_pSceneRTTV->Release();
			m_pSceneRTTV = nullptr;
		}

		if (m_pSceneSRV != nullptr)
		{
			m_pSceneSRV->Release();
			m_pSceneSRV = nullptr;
		}

		if (m_pSSAORTT != nullptr)
		{
			m_pSSAORTT->Release();
			m_pSSAORTT = nullptr;
		}

		if (m_pSSAORTTV != nullptr)
		{
			m_pSSAORTTV->Release();
			m_pSSAORTTV = nullptr;
		}

		if (m_pSSAOSRV != nullptr)
		{
			m_pSSAOSRV->Release();
			m_pSSAOSRV = nullptr;
		}
		
		if (m_pSSAOBlurRTT != nullptr)
		{
			m_pSSAOBlurRTT->Release();
			m_pSSAOBlurRTT = nullptr;
		}

		if (m_pSSAOBlurRTTV != nullptr)
		{
			m_pSSAOBlurRTTV->Release();
			m_pSSAOBlurRTTV = nullptr;
		}

		if (m_pSSAOBlurSRV != nullptr)
		{
			m_pSSAOBlurSRV->Release();
			m_pSSAOBlurSRV = nullptr;
		}

		if (m_pBrightRTT != nullptr)
		{
			m_pBrightRTT->Release();
			m_pBrightRTT = nullptr;
		}


		if (m_pBrightRTTV != nullptr)
		{
			m_pBrightRTTV->Release();
			m_pBrightRTTV = nullptr;
		}


		if (m_pBrightSRV != nullptr)
		{
			m_pBrightSRV->Release();
			m_pBrightSRV = nullptr;
		}

		if (m_pViewportRTT != nullptr)
		{
			m_pViewportRTT->Release();
			m_pViewportRTT = nullptr;
		}

		if (m_pViewportRTTV != nullptr)
		{
			m_pViewportRTTV->Release();
			m_pViewportRTTV = nullptr;
		}

		if (m_pViewportSRV != nullptr)
		{
			m_pViewportSRV->Release();
			m_pViewportSRV = nullptr;
		}

		for (int i = 0; i < 2; ++i)
		{
			if (m_pPingPongRTT[i] != nullptr)
			{
				m_pPingPongRTT[i]->Release();
				m_pPingPongRTT[i] = nullptr;
			}

			if (m_pPingPongRTTV[i] != nullptr)
			{
				m_pPingPongRTTV[i]->Release();
				m_pPingPongRTTV[i] = nullptr;
			}

			if (m_pPingPongSRV[i] != nullptr)
			{
				m_pPingPongSRV[i]->Release();
				m_pPingPongSRV[i] = nullptr;
			}
		}
		for (int i = 0; i < _GBUF_CNT; ++i)
		{
			if (m_pGbufferRTT[i] != nullptr)
			{
				m_pGbufferRTT[i]->Release();
				m_pGbufferRTT[i]= nullptr;
			}

			if (m_pGbufferRTTV[i] != nullptr)
			{
				m_pGbufferRTTV[i]->Release();
				m_pGbufferRTTV[i] = nullptr;
			}

			if (m_pGbufferSRV[i] != nullptr)
			{
				m_pGbufferSRV[i]->Release();
				m_pGbufferSRV[i] = nullptr;
			}
		}
	}
	void D3D11::DestroyDSBforRTT()
	{
		if (m_pDSBufferForRTT != nullptr)
		{
			m_pDSBufferForRTT->Release();
			m_pDSBufferForRTT = nullptr;
		}

		if (m_pSceneDSV != nullptr)
		{
			m_pSceneDSV->Release();
			m_pSceneDSV = nullptr;
		}
	}
	
}