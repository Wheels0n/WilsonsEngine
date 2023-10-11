#define STB_IMAGE_IMPLEMENTATION
#include <random>
#include "D3D11.h"
#include "../ImGui/imgui_impl_dx11.h"
#include "stb_image.h"

namespace wilson
{
	D3D11::D3D11(int screenWidth, int screenHeight, bool bVsync, HWND hWnd, bool bFullscreen,
		float fScreenFar, float fScreenNear)
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
		m_pHeightScaleBuffer = nullptr;
		m_pHeightOnOffBuffer = nullptr;
		m_pEquirect2CubeBuffer = nullptr;
		m_pAABBVBuffer = nullptr;
		m_pAABBIBuffer = nullptr;

		m_pScreenRTTV = nullptr;
		m_pScreenDepthRTT = nullptr;
		m_pSceneDepthRTT = nullptr;
		m_pOutlinerSetupDSS = nullptr;
		m_pOutlinerTestDSS = nullptr;

		m_pSSAORTT = nullptr;
		m_pSSAORTTV = nullptr;
		m_pSSAOSRV = nullptr;
		m_pSSAOBlurRTT = nullptr;
		m_pSSAOBlurRTTV = nullptr;
		m_pSSAOBlurSRV = nullptr;

		m_pSSAOBlurDebugRTT = nullptr;
		m_pSSAOBlurDebugRTTV = nullptr;
		m_pSSAOBlurDebugSRV = nullptr;

		m_pViewportRTT = nullptr;
		m_pViewportRTTV = nullptr;
		m_pViewportSRV = nullptr;
		m_pSceneRTT = nullptr;
		m_pSceneRTTV = nullptr;
		m_pSceneSRV = nullptr;
		
		m_pBrightRTT = nullptr;
		m_pBrightRTTV = nullptr;
		m_pBrightSRV = nullptr;

		m_pSkyBoxRTT = nullptr;
		m_pSkyBoxRTTV = nullptr;
		m_pSkyBoxSRV = nullptr;

		m_pDiffIrradianceRTT = nullptr;
		m_pDiffIrradianceRTTV = nullptr;
		m_pDiffIrradianceSRV = nullptr;

		m_pPrefilterRTT = nullptr;
		m_pPrefilterRTTV = nullptr;
		m_pPrefilterSRV = nullptr;

		m_pBRDFRTT = nullptr;
		m_pBRDFRTTV = nullptr;
		m_pBRDFSRV = nullptr;

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

		m_pHDRRTT = nullptr;
		m_pHDRSRV = nullptr;

		m_viewport = {};
		m_prefilterViewport = {};
		m_diffIrradViewport = {};

		m_pQuadRS = nullptr;
		m_pGeoRS = nullptr;
		m_pAABBRS = nullptr;

		m_pRasterStateCC = nullptr;

		m_pWrapSS = nullptr;
		m_pClampSS = nullptr;

		m_pGBufferWriteBS = nullptr;
		m_pLightingPassBS = nullptr;

		m_pCam = nullptr;
		m_pFrustum = nullptr;
		m_pLightBuffer = nullptr;
		m_pMatBuffer = nullptr;
		m_pShader = nullptr;
		m_pShadowMap = nullptr;

		m_clientWidth = 0;
		m_clientHeight = 0;
		
		m_selectedModelGroup = -1;
		m_selectedModel = -1;

		m_bVsyncOn = false;
		m_bAABBGridOn=false;
		m_bHeightOnOFF = FALSE;

		m_exposure = 1.0f;
		m_heightScale = 0.0001f;

		{
			HRESULT hr;
			bool result;
			IDXGIFactory* pFactory;
			IDXGIAdapter* pAdapter;
			IDXGIOutput* pAdapterOutput;
			UINT numModes = 0, i = 0, numerator = 0, denominator = 0;
			size_t strLen = 0;
			DXGI_MODE_DESC* pDisplayModeList = {};
			DXGI_ADAPTER_DESC adapterDesc = {};
			int iError = 0;
			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			D3D_FEATURE_LEVEL featureLevel = {};
			ID3D11Texture2D* pBackbuffer = {};
			D3D11_RASTERIZER_DESC rasterDesc = {};
			D3D11_RENDER_TARGET_BLEND_DESC rtBlendDSC = {};
			float fFOV = 0.0f, fScreenAspect = 0.0f;
			m_bVsyncOn = bVsync;

			hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pFactory));
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::pFactory::CreateDXGIFactoryFailed");
			}
			pFactory->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::pFactory") - 1, "D3D11::pFactory");

			//Enumerates video cards
			hr = pFactory->EnumAdapters(0, &pAdapter);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::pAdapter::EnumAdaptersFailed");
			}
			pAdapter->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::pAdapter") - 1, "D3D11::pAdapter");

			//Enumerates outputs(ex:monitor)
			hr = pAdapter->EnumOutputs(0, &pAdapterOutput);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::pAdapter::EnumOutputsFailed");
			}

			//Each monitor has a set of display modes it supports. A display mode refers to the following data in DXGI_MODE_DESC
			hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::pAdapterOutput::GetNumOfDisplayModeListFailed");
			}

			//Fixing a display mode format, we can get a list of all supported display modes an output supports in that format with the following code :
			pDisplayModeList = new DXGI_MODE_DESC[numModes];
			if (pDisplayModeList == nullptr)
			{
				OutputDebugStringA("D3D11::CreateDXGI_MODE_DESC Failed");
			}

			hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, pDisplayModeList);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::pAdapterOutput::GetDisplayModeListFailed");
			}

			hr = pAdapter->GetDesc(&adapterDesc);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::pAdapter::GetDescFailed");
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
				OutputDebugStringA("D3D11::Create Device, Context, and SwapChain Failed");
			}
			m_pDevice->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pDevice") - 1, "D3D11::m_pDevice");
			m_pContext->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pContext") - 1, "D3D11::m_pContext");
			m_pSwapChain->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pSwapChain") - 1, "D3D11::m_pSwapChain");




			hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackbuffer));
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::pBackbuffer::CreateBackBufferFailed");
			}
			pBackbuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::pBackbuffer") - 1, "D3D11::pBackbuffer");

			hr = m_pDevice->CreateRenderTargetView(pBackbuffer, nullptr, &m_pScreenRTTV);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::m_pScreenRTTV::CreateRTVFailed");
			}
			m_pScreenRTTV->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pScreenRTTV") - 1, "D3D11::m_pScreenRTTV");


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

				D3D11_BUFFER_DESC quadBDSC = { 0, };
				quadBDSC.Usage = D3D11_USAGE_DEFAULT;
				quadBDSC.ByteWidth = sizeof(QUAD) * 4;
				quadBDSC.BindFlags = D3D11_BIND_VERTEX_BUFFER;


				hr = m_pDevice->CreateBuffer(&quadBDSC, &quadData, &m_pQuadVB);
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pQuadVB::CreateVBFailed");
				}
				m_pQuadVB->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pQuadVB") - 1, "D3D11::m_pQuadVB");


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
					OutputDebugStringA("D3D11::m_pQuadIB::CreateIBFailed");
				}
				m_pQuadIB->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pQuadIB") - 1, "D3D11::m_pQuadIB");

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

				hr = m_pDevice->CreateBuffer(&skyBoxVertexBD, &skyBoxVertexData, &m_pCubeVertices);
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pCubeVertices::CreateVBFailed");
				}
				m_pCubeVertices->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pCubeVertices") - 1, "D3D11::m_pCubeVertices");

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
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pCubeIndices::CreateIBFailed");
				}
				m_pCubeIndices->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pCubeIndices") - 1, "D3D11::m_pCubeIndices");


				D3D11_DEPTH_STENCIL_DESC skyboxDSD;
				ZeroMemory(&skyboxDSD, sizeof(D3D11_DEPTH_STENCIL_DESC));
				skyboxDSD.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
				hr = m_pDevice->CreateDepthStencilState(&skyboxDSD, &m_pSkyBoxDSS);
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pSkyBoxDSS::CreateDSSFailed");
				}
				m_pSkyBoxDSS->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pSkyBoxDSS") - 1, "D3D11::m_pSkyBoxDSS");


				D3D11_RASTERIZER_DESC skyboxRD;
				ZeroMemory(&skyboxRD, sizeof(D3D11_RASTERIZER_DESC));
				skyboxRD.FillMode = D3D11_FILL_SOLID;
				skyboxRD.CullMode = D3D11_CULL_BACK;
				hr = m_pDevice->CreateRasterizerState(&skyboxRD, &m_pSkyBoxRS);
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pSkyBoxRS::CreateRSFailed");
				}
				m_pSkyBoxRS->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pSkyBoxRS") - 1, "D3D11::m_pSkyBoxRS");

			}

			if (!CreateRTT(m_clientWidth, m_clientHeight))
			{
				OutputDebugStringA("D3D11::CreateRTT()Failed");
			}

			if (!CreateDSS())
			{
				OutputDebugStringA("D3D11::CreateDSS()Failed");
			}

			if (!CreateDepthBuffer(m_clientWidth, m_clientHeight, &m_pScreenDepthRTT, &m_pScreenDSV))
			{
				OutputDebugStringA("D3D11::CreateDepthBuffer()onScreenFailed");
			}

			if (!CreateDepthBuffer(m_clientWidth, m_clientHeight, &m_pSceneDepthRTT, &m_pSceneDSV))
			{
				OutputDebugStringA("D3D11::CreateDepthBuffer()onSceneFailed");
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
				OutputDebugStringA("D3D11::m_pGeoRS::CreateRSFailed");
			}
			m_pGeoRS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pGeoRS") - 1, "D3D11::m_pGeoRS");

			rasterDesc.FrontCounterClockwise = false;
			hr = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pQuadRS);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::m_pQuadRS::CreateRSFailed");
			}
			m_pQuadRS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pQuadRS") - 1, "D3D11::m_pQuadRS");

			rasterDesc.FrontCounterClockwise = false;
			rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
			hr = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pAABBRS);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::m_pAABBRS::CreateRSFailed");
			}
			m_pAABBRS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pAABBRS") - 1, "D3D11::m_pAABBRS");

			m_viewport.Width = static_cast<float>(screenWidth);
			m_viewport.Height = static_cast<float>(screenHeight);
			m_viewport.MinDepth = 0.0f;
			m_viewport.MaxDepth = 1.0f;
			m_viewport.TopLeftX = 0.0f;
			m_viewport.TopLeftY = 0.0f;

			m_diffIrradViewport = m_viewport;
			m_diffIrradViewport.Width = 32;
			m_diffIrradViewport.Height = 32;

			m_prefilterViewport = m_viewport;
			m_prefilterViewport.Width = 128;
			m_prefilterViewport.Height = 128;

			D3D11_SAMPLER_DESC samplerDesc = {};
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

			hr = m_pDevice->CreateSamplerState(&samplerDesc, &m_pWrapSS);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::m_pWrapSS::CreateSSFailed");
			}
			m_pWrapSS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pWrapSS") - 1, "D3D11::m_pWrapSS");

			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			hr = m_pDevice->CreateSamplerState(&samplerDesc, &m_pClampSS);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::m_pClampSS::CreateSSFailed");
			}
			m_pClampSS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pClampSS") - 1, "D3D11::m_pClampSS");

			rtBlendDSC.BlendEnable = TRUE;
			rtBlendDSC.SrcBlend = D3D11_BLEND_ONE;
			rtBlendDSC.DestBlend = D3D11_BLEND_ZERO;
			rtBlendDSC.BlendOp = D3D11_BLEND_OP_ADD;
			rtBlendDSC.SrcBlendAlpha = D3D11_BLEND_ONE;
			rtBlendDSC.DestBlendAlpha = D3D11_BLEND_ZERO;
			rtBlendDSC.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			rtBlendDSC.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			D3D11_BLEND_DESC blendDSC = { FALSE, FALSE, rtBlendDSC };
			hr = m_pDevice->CreateBlendState(&blendDSC, &m_pGBufferWriteBS);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::m_pGBufferWriteBS::CreateBSFailed");
			}
			m_pGBufferWriteBS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pGBufferWriteBS") - 1, "D3D11::m_pGBufferWriteBS");


			rtBlendDSC.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDSC.RenderTarget[0] = rtBlendDSC;
			hr = m_pDevice->CreateBlendState(&blendDSC, &m_pLightingPassBS);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::m_pLightingPassBS::CreateBSFailed");
			}
			m_pLightingPassBS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pLightingPassBS") - 1, "D3D11::m_pLightingPassBS");


			//Set projectionMatrix, viewMatrix;
			m_pCam = new Camera(m_pDevice, screenWidth, screenHeight, fScreenFar, fScreenNear);
			m_pCam->SetCamPos(m_pContext);
			XMMATRIX* m_projMat = m_pCam->GetProjectionMatrix();
			XMMATRIX* m_viewMat = m_pCam->GetViewMatrix();
			m_pFrustum = new Frustum(m_pCam);
			m_pMatBuffer = new MatBuffer(m_pDevice, m_pContext, m_viewMat, m_projMat);

			m_pLightBuffer = new LightBuffer(m_pDevice);

			m_pShader = new Shader(m_pDevice, m_pContext);

			m_pShadowMap = new ShadowMap(m_pDevice, _SHADOWMAP_SIZE, _SHADOWMAP_SIZE, m_pCam->GetCascadeLevels().size(),
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
					OutputDebugStringA("D3D11::m_pBoolBuffer::CreateBufferFailed");
				}
				m_pBoolBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pBoolBuffer") - 1, "D3D11::m_pBoolBuffer");

				hr = m_pDevice->CreateBuffer(&bds, 0, &m_pHeightOnOffBuffer);
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pHeightOnOffBuffer::CreateBufferFailed");
				}
				m_pHeightOnOffBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pHeightOnOffBuffer") - 1, "D3D11::m_pHeightOnOffBuffer");


				bds.ByteWidth = sizeof(XMVECTOR);
				hr = m_pDevice->CreateBuffer(&bds, 0, &m_pColorBuffer);
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pColorBuffer::CreateBufferFailed");
				}
				m_pColorBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pColorBuffer") - 1, "D3D11::m_pColorBuffer");

				bds.ByteWidth = sizeof(SamplePoints);
				hr = m_pDevice->CreateBuffer(&bds, 0, &m_pSSAOKernelBuffer);
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pSSAOKernel::CreateBufferFailed");
				}
				m_pSSAOKernelBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pSSAOKernelBuffer") - 1, "D3D11::m_pSSAOKernelBuffer");

				bds.ByteWidth = sizeof(XMFLOAT4);
				hr = m_pDevice->CreateBuffer(&bds, 0, &m_pExposureBuffer);
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pExposureBuffer::CreateBufferFailed");
				}
				m_pExposureBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pExposureBuffer") - 1, "D3D11::m_pExposureBuffer");


				hr = m_pDevice->CreateBuffer(&bds, 0, &m_pHeightScaleBuffer);
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pHeightScaleBuffer::CreateBufferFailed");
				}
				m_pHeightScaleBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pHeightScaleBuffer") - 1, "D3D11::m_pHeightScaleBuffer");


				bds.ByteWidth = sizeof(XMMATRIX) * 6;
				hr = m_pDevice->CreateBuffer(&bds, 0, &m_pEquirect2CubeBuffer);
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pEquirect2CubeBuffer::CreateBufferFailed");
				}
				m_pEquirect2CubeBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pEquirect2CubeBuffer") - 1, "D3D11::m_pEquirect2CubeBuffer");

				bds.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bds.ByteWidth = sizeof(DirectX::XMFLOAT3) * 8;
				hr = m_pDevice->CreateBuffer(&bds, 0, &m_pAABBVBuffer);
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pAABBVBuffer::CreateVBFailed");
				}
				m_pAABBVBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pAABBVBuffer") - 1, "D3D11::m_pAABBVBuffer");


				unsigned long cubeIndices[24] = { 0, 1,   1, 2,   2, 3,  3,0 ,
						   0, 4,   1, 5,   2, 6,  3,7,
						   4, 5,   5, 6,   6, 7,  7,0 };
				D3D11_SUBRESOURCE_DATA cubeIndexData = {};
				cubeIndexData.pSysMem = cubeIndices;
				cubeIndexData.SysMemPitch = 0;
				cubeIndexData.SysMemSlicePitch = 0;

				bds.Usage = D3D11_USAGE_DEFAULT;
				bds.BindFlags = D3D11_BIND_INDEX_BUFFER;
				bds.ByteWidth = sizeof(unsigned long) * 24;
				bds.CPUAccessFlags = 0;
				hr = m_pDevice->CreateBuffer(&bds, &cubeIndexData, &m_pAABBIBuffer);
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pAABBIBuffer::CreateIBFailed");
				}
				m_pAABBIBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pAABBIBuffer") - 1, "D3D11::m_pAABBIBuffer");
			}
			{

				//Gen Sample points
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				SamplePoints* pSamplePoints;
				hr = m_pContext->Map(m_pSSAOKernelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pSSAOKernelBuffer::MapFailed");
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
					OutputDebugStringA("D3D11::m_pNoiseRTT::CreateTexFailed");
				}
				m_pNoiseRTT->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pNoiseRTT") - 1, "D3D11::m_pNoiseRTT");

				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Format = texDesc.Format;
				srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2D.MostDetailedMip = 0;
				hr = m_pDevice->CreateShaderResourceView(m_pNoiseRTT, &srvDesc, &m_pNoiseSRV);
				if (FAILED(hr))
				{
					OutputDebugStringA("D3D11::m_pNoiseSRV::CreateSRVFailed");
				}
				m_pNoiseSRV->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pNoiseSRV") - 1, "D3D11::m_pNoiseSRV");
			}
			//Gen Equirectangular map
			{
				int width, height, nrComponents;
				float* data = stbi_loadf(".\\Assets\\Models\\FBX\\Bistro_v5_2\\Bistro_v5_2\\san_giuseppe_bridge_4k.hdr",
					&width, &height, &nrComponents, STBI_rgb_alpha);
				if (data)
				{
					D3D11_TEXTURE2D_DESC texDesc = {};
					texDesc.Width = width;
					texDesc.Height = height;
					texDesc.ArraySize = 1;
					texDesc.MipLevels = 1;
					texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					texDesc.SampleDesc.Count = 1;
					texDesc.SampleDesc.Quality = 0;
					texDesc.Usage = D3D11_USAGE_DEFAULT;
					texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
					texDesc.CPUAccessFlags = 0;
					texDesc.MiscFlags = 0;

					D3D11_SUBRESOURCE_DATA subResource = {};
					subResource.pSysMem = data;
					subResource.SysMemPitch = sizeof(float) * width * 4;

					hr = m_pDevice->CreateTexture2D(&texDesc, &subResource, &m_pHDRRTT);
					stbi_image_free(data);
					if (FAILED(hr))
					{
						OutputDebugStringA("D3D11::m_pHDRRTT::CreateTexFailed");
					}
					m_pHDRRTT->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("D3D11::m_pHDRRTT") - 1, "D3D11::m_pHDRRTT");


					D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.Format = texDesc.Format;
					srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
					srvDesc.Texture2D.MostDetailedMip = 0;
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					hr = m_pDevice->CreateShaderResourceView(m_pHDRRTT, &srvDesc, &m_pHDRSRV);
					if (FAILED(hr))
					{
						OutputDebugStringA("D3D11::m_pHDRSRV::CreateSRVFailed");
					}
					m_pHDRSRV->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("D3D11::m_pHDRSRV") - 1, "D3D11::m_pHDRSRV");


					//Covenrt EquirectangularMap to CubeMap
					D3D11_MAPPED_SUBRESOURCE mappedSubResource;
					hr = m_pContext->Map(m_pEquirect2CubeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource);
					if (FAILED(hr))
					{
						OutputDebugStringA("D3D11::m_pEquirect2CubeBuffer::MapFailed");
					}
					UINT stride;
					UINT offset = 0;

					XMMATRIX capProj = XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f),
						1.0F, 0.1f, 10.0f);
					XMVECTOR eyePos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
					XMMATRIX capView[6] =
					{	//변환된 텍스쳐가 뒤집혀져있음에 유의
						XMMatrixLookAtLH(eyePos, XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
						XMMatrixLookAtLH(eyePos, XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
						XMMatrixLookAtLH(eyePos, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f)),
						XMMatrixLookAtLH(eyePos, XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)),
						XMMatrixLookAtLH(eyePos, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
						XMMatrixLookAtLH(eyePos, XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
					};
					XMMATRIX* pMatrix = (XMMATRIX*)mappedSubResource.pData;
					for (int i = 0; i < 6; ++i)
					{
						pMatrix[i] = XMMatrixMultiplyTranspose(capView[i], capProj);

					}
					m_pContext->Unmap(m_pEquirect2CubeBuffer, 0);


					stride = sizeof(XMFLOAT3);
					m_pShader->SetPosOnlyInputLayout(m_pContext);
					m_pShader->SetEquirect2CubeShader(m_pContext);
					m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					m_pContext->IASetVertexBuffers(0, 1, &m_pCubeVertices, &stride, &offset);
					m_pContext->IASetIndexBuffer(m_pCubeIndices, DXGI_FORMAT_R32_UINT, 0);
					m_pContext->GSSetConstantBuffers(0, 1, &m_pEquirect2CubeBuffer);
					m_pContext->PSSetShaderResources(0, 1, &m_pHDRSRV);
					m_pContext->PSSetSamplers(0, 1, &m_pWrapSS);
					m_pContext->RSSetViewports(1, m_pShadowMap->GetViewport());
					m_pContext->OMSetDepthStencilState(0, 0);
					m_pContext->OMSetRenderTargets(1, &m_pSkyBoxRTTV, nullptr);
					m_pContext->DrawIndexed(36, 0, 0);
					//Gen Diffuse IrradianceMap
					m_pShader->SetDiffuseIrradianceShader(m_pContext);
					m_pContext->OMSetRenderTargets(1, &m_pDiffIrradianceRTTV, nullptr);
					m_pContext->PSSetShaderResources(0, 1, &m_pSkyBoxSRV);
					m_pContext->PSSetSamplers(0, 1, &m_pClampSS);
					m_pContext->RSSetViewports(1, &m_diffIrradViewport);
					m_pContext->DrawIndexed(36, 0, 0);
					//Gen PrefileterMap;
					m_pShader->SetPrefilterShader(m_pContext);
					m_pContext->PSSetSamplers(0, 1, &m_pWrapSS);
					m_pContext->RSSetViewports(1, &m_prefilterViewport);
					m_pContext->OMSetRenderTargets(1, &m_pPrefilterRTTV, nullptr);
					m_pContext->DrawIndexed(36, 0, 0);
					m_pContext->GenerateMips(m_pPrefilterSRV);
					//Gen BRDFMap
					m_pShader->SetTexInputlayout(m_pContext);
					m_pShader->SetBRDFShader(m_pContext);
					stride = sizeof(QUAD);
					m_pContext->IASetVertexBuffers(0, 1, &m_pQuadVB, &stride, &offset);
					m_pContext->IASetIndexBuffer(m_pQuadIB, DXGI_FORMAT_R32_UINT, 0);
					m_pContext->RSSetViewports(1, m_pShadowMap->GetViewport());
					m_pContext->PSSetSamplers(0, 1, &m_pClampSS);
					m_pContext->OMSetRenderTargets(1, &m_pBRDFRTTV, nullptr);
					m_pContext->DrawIndexed(6, 0, 0);
				}

			}
			ImGui_ImplDX11_Init(m_pDevice, m_pContext);
		}
	}

	D3D11::~D3D11()
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
		if (m_pAABBRS != nullptr)
		{	
			m_pAABBRS->Release();
			m_pAABBRS = nullptr;
		}


		if (m_pRasterStateCC != nullptr)
		{
			m_pRasterStateCC->Release();
			m_pRasterStateCC = nullptr;
		}

		DestroyRTT();
		DestroyDSBforRTT();
		if (m_pScreenDepthRTT != nullptr)
		{
			m_pScreenDepthRTT->Release();
			m_pScreenDepthRTT = nullptr;
		}

		if (m_pScreenDSV != nullptr)
		{
			m_pScreenDSV->Release();
			m_pScreenDSV = nullptr;
		}

		DestroyDSS();
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


		if (m_pHDRRTT != nullptr)
		{
			m_pHDRRTT->Release();
			m_pHDRRTT = nullptr;
		}
		if (m_pHDRSRV != nullptr)
		{
			m_pHDRSRV->Release();
			m_pHDRSRV = nullptr;
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

		if (m_pWrapSS != nullptr)
		{
			m_pWrapSS->Release();
			m_pWrapSS = nullptr;
		}

		if (m_pClampSS != nullptr)
		{
			m_pClampSS->Release();
			m_pClampSS = nullptr;
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

		if (m_pHeightScaleBuffer != nullptr)
		{
			m_pHeightScaleBuffer->Release();
			m_pHeightScaleBuffer = nullptr;
		}

		if (m_pHeightOnOffBuffer != nullptr)
		{
			m_pHeightOnOffBuffer->Release();
			m_pHeightOnOffBuffer = nullptr;
		}
		if (m_pEquirect2CubeBuffer != nullptr)
		{
			m_pEquirect2CubeBuffer->Release();
			m_pEquirect2CubeBuffer = nullptr;
		}

		if (m_pAABBVBuffer != nullptr)
		{
			m_pAABBVBuffer->Release();
			m_pAABBVBuffer = nullptr;
		}
		if (m_pAABBIBuffer != nullptr)
		{
			m_pAABBIBuffer->Release();
			m_pAABBIBuffer = nullptr;
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
		UINT texCnt = _GBUF_CNT-2;
		UINT stride;
		UINT offset = 0;
		int drawed = 0;
		bool bGeoPass = false;
		
		ID3D11RenderTargetView* nullRTV[_GBUF_CNT] = { nullptr, };
		ID3D11RenderTargetView* pSSAORTVs[2] = {m_pSSAOBlurRTTV, m_pSSAOBlurDebugRTTV};

		m_pContext->ClearRenderTargetView(m_pScreenRTTV, color);
		m_pContext->ClearRenderTargetView(m_pViewportRTTV, color);
		m_pContext->ClearRenderTargetView(m_pSceneRTTV, color);
		m_pContext->ClearRenderTargetView(m_pSSAORTTV, color);
		m_pContext->ClearRenderTargetView(m_pSSAOBlurRTTV, color);
		m_pContext->ClearRenderTargetView(m_pSSAOBlurDebugRTTV, color);
		for (int i = 0; i < _GBUF_CNT; ++i)
		{
			m_pContext->ClearRenderTargetView(m_pGbufferRTTV[i], color);
		}

		m_pContext->ClearDepthStencilView(m_pScreenDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_pContext->ClearDepthStencilView(m_pSceneDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_pContext->PSSetSamplers(0, 1, &m_pWrapSS);
		m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pCam->Update();
		//Update Light
		std::vector<DirectionalLight*>& dirLights = m_pLightBuffer->GetDirLights();
		std::vector<PointLight*>& pointLights = m_pLightBuffer->GetPointLights();
		std::vector<SpotLight*>& spotLights = m_pLightBuffer->GetSpotLights();
		//Draw ShadowMap
		{	
			m_pContext->PSSetShaderResources(0, 4 +dirLights.capacity()+ pointLights.capacity()+ spotLights.capacity(), m_pLightBuffer->GetNullSRVs());
			m_pContext->RSSetViewports(1, m_pShadowMap->GetViewport());
			m_pContext->RSSetState(m_pQuadRS);
			m_pContext->OMSetDepthStencilState(0, 0);
			
		
			m_pShader->SetPosOnlyInputLayout(m_pContext);
			m_pShader->SetCascadeDirShadowShader(m_pContext);
			for (int i = 0; i < dirLights.size(); ++i)
			{	
				dirLights[i]->UpdateLightSpaceMatrices();
				dirLights[i]->SetShadowMatrices(m_pContext);
				m_pShadowMap->BindDirDSV(m_pContext,i);
				DrawENTT(bGeoPass);
			}

			m_pShader->SetTexInputlayout(m_pContext);
			m_pShader->SetSpotShadowShader(m_pContext);
			for (int i = 0; i < spotLights.size(); ++i)
			{
				m_pMatBuffer->SetLightSpaceMatrix(spotLights[i]->GetLightSpaceMat());
				m_pShadowMap->BindSpotDSV(m_pContext, i);
				DrawENTT(bGeoPass);
			}


			m_pContext->RSSetState(m_pSkyBoxRS);
			m_pShader->SetOmniDirShadowShader(m_pContext);
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
		m_pMatBuffer->Update(m_pContext);
		m_pShader->SetSkyBoxShader(m_pContext);
		m_pContext->IASetVertexBuffers(0, 1, &m_pCubeVertices, &stride, &offset);
		m_pContext->IASetIndexBuffer(m_pCubeIndices, DXGI_FORMAT_R32_UINT, 0);
		m_pContext->PSSetShaderResources(0, 1, &m_pSkyBoxSRV);
		m_pContext->PSSetShaderResources(1, 1, &m_pHDRSRV);
		m_pContext->RSSetViewports(1, &m_viewport);
		m_pContext->OMSetDepthStencilState(m_pSkyBoxDSS, 0);
		m_pContext->OMSetRenderTargets(1, &m_pGbufferRTTV[2], m_pSceneDSV);
		m_pContext->DrawIndexed(36, 0, 0);
	
		//Deferred Shading First Pass
		if (!m_pModelGroups.empty())
		{	
			m_pCam->SetCamPos(m_pContext);
			m_pShader->SetDeferredGeoLayout(m_pContext);
			m_pShader->SetPBRDeferredGeoShader(m_pContext);
			m_pContext->RSSetState(m_pQuadRS);
			m_pContext->OMSetDepthStencilState(0, 0);
			m_pContext->OMSetBlendState(m_pGBufferWriteBS, color, 0xffffffff);
			m_pContext->OMSetRenderTargets(_GBUF_CNT, m_pGbufferRTTV, m_pSceneDSV);

			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				XMFLOAT4* pHeightScale;
				m_pContext->Map(m_pHeightScaleBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				pHeightScale = (XMFLOAT4*)mappedResource.pData;
				pHeightScale->x = m_heightScale;
				m_pContext->Unmap(m_pHeightScaleBuffer, 0);
				m_pContext->PSSetConstantBuffers(3, 1, &m_pHeightScaleBuffer);

				BOOL* pHeightOnOFF;
				m_pContext->Map(m_pHeightOnOffBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				pHeightOnOFF = (BOOL*)mappedResource.pData;
				*pHeightOnOFF = m_bHeightOnOFF;
				m_pContext->Unmap(m_pHeightOnOffBuffer, 0);
				m_pContext->PSSetConstantBuffers(4, 1, &m_pHeightOnOffBuffer);

			}
			DrawENTT(!bGeoPass);
			

			//SSAO Pass
			stride = sizeof(QUAD);
			m_pContext->IASetVertexBuffers(0, 1, &m_pQuadVB, &stride, &offset);
			m_pContext->IASetIndexBuffer(m_pQuadIB, DXGI_FORMAT_R32_UINT, 0);

			m_pShader->SetTexInputlayout(m_pContext);
			m_pShader->SetSSAOShader(m_pContext);

			m_pContext->PSSetConstantBuffers(0, 1, &m_pSSAOKernelBuffer);
			m_pMatBuffer->UploadProjMat(m_pContext);
			m_pContext->OMSetRenderTargets(1, &m_pSSAORTTV, nullptr);
			m_pContext->PSSetShaderResources(0, 2, &m_pGbufferSRV[_GBUF_CNT-3]);
			m_pContext->PSSetShaderResources(2, 1, &m_pNoiseSRV);
			m_pContext->PSSetSamplers(0, 1, &m_pWrapSS);
			m_pContext->PSSetSamplers(1, 1, &m_pClampSS);
			m_pContext->DrawIndexed(6, 0, 0);
			//Blur SSAOTex
			m_pShader->SetSSAOBlurShader(m_pContext);
			m_pContext->OMSetRenderTargets(2, pSSAORTVs, nullptr);
			m_pContext->PSSetShaderResources(0, 1, &m_pSSAOSRV);
			m_pContext->DrawIndexed(6, 0, 0);

			//Lighting Pass
			m_pShader->SetTexInputlayout(m_pContext);
			m_pShader->SetPBRDeferredLightingShader(m_pContext);
			m_pLightBuffer->UpdateDirLightMatrices(m_pContext);
			m_pLightBuffer->UpdateSpotLightMatrices(m_pContext);
			m_pLightBuffer->UpdateLightBuffer(m_pContext);
			m_pCam->SetCamPos(m_pContext);
			m_pCam->SetCascadeLevels(m_pContext);
	
			m_pContext->OMSetRenderTargets(1, &m_pSceneRTTV, nullptr);
			m_pContext->OMSetBlendState(m_pLightingPassBS, color, 0xffffffff);
			m_pContext->PSSetShaderResources(0, _GBUF_CNT-2, m_pGbufferSRV);
			m_pContext->PSSetShaderResources(texCnt++, 1, &m_pSSAOBlurSRV);
			m_pContext->PSSetShaderResources(texCnt++, 1, &m_pDiffIrradianceSRV);
			m_pContext->PSSetShaderResources(texCnt++, 1, &m_pPrefilterSRV);
			m_pContext->PSSetShaderResources(texCnt++, 1, &m_pBRDFSRV);
			
			m_pContext->PSSetShaderResources(texCnt, dirLights.capacity(), m_pShadowMap->GetDirSRV());
			m_pContext->PSSetShaderResources(texCnt +dirLights.capacity(), spotLights.capacity(), m_pShadowMap->GetSpotSRV());
			m_pContext->PSSetShaderResources(texCnt +dirLights.capacity() + spotLights.capacity(), pointLights.capacity(), m_pShadowMap->GetCubeSRV());
	

			m_pContext->PSSetShaderResources(texCnt++, 1, m_pShadowMap->GetDirSRV());
			m_pContext->PSSetShaderResources(texCnt++, 1, m_pShadowMap->GetSpotSRV());
			m_pContext->PSSetShaderResources(texCnt, 1, m_pShadowMap->GetCubeSRV());
			m_pContext->PSSetSamplers(1, 1, m_pShadowMap->GetCubeShadowSampler());
			m_pContext->PSSetSamplers(2, 1, m_pShadowMap->GetDirShadowSampler());
			m_pContext->DrawIndexed(6, 0, 0);
			
		}
		
		//Draw picked Model's Outline
		m_pShader->SetTexInputlayout(m_pContext);
		m_pShader->SetOutlinerShader(m_pContext);
		m_pContext->OMSetDepthStencilState(m_pOutlinerTestDSS, 1);
		m_pContext->OMSetRenderTargets(1, &m_pSceneRTTV, m_pSceneDSV);
		if (m_selectedModelGroup != -1)
		{
			std::vector<Model*> pModels = m_pModelGroups[m_selectedModelGroup]->GetModels();
			XMMATRIX worldMat = pModels[m_selectedModel]->GetTransformMatrix(true);
			m_pMatBuffer->SetWorldMatrix(&worldMat);
			m_pMatBuffer->Update(m_pContext);

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
		m_pShader->SetTexInputlayout(m_pContext);
		m_pShader->SetFinalShader(m_pContext);
		m_pContext->RSSetState(m_pQuadRS);
		m_pContext->OMSetRenderTargets(0, nullptr, nullptr);
		{	
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			XMFLOAT4* pExposure;
			m_pContext->Map(m_pExposureBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			pExposure = (XMFLOAT4*)mappedResource.pData;
			pExposure->x = m_exposure;
			m_pContext->Unmap(m_pExposureBuffer, 0);
			m_pContext->PSSetConstantBuffers(0, 1, &m_pExposureBuffer);
		}
		
		m_pContext->PSSetShaderResources(0, 1, m_pModelGroups.empty()||!m_pFrustum->GetENTTsInFrustum() ? &m_pGbufferSRV[2] : &m_pSceneSRV);
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
		switch (pLight->GetType())
		{
		case eLIGHT_TYPE::DIR:
			m_pLightBuffer->PushDirLight((DirectionalLight*)pLight);
			break;
		case eLIGHT_TYPE::PNT:
			m_pLightBuffer->PushPointLight((PointLight*)pLight);
			break;
		case eLIGHT_TYPE::SPT:
			m_pLightBuffer->PushSpotLight((SpotLight*)pLight);
		}
	}
	void D3D11::AddModelGroup(ModelGroup* pModelGroup, ID3D11Device* pDevice)
	{
		m_pModelGroups.push_back(pModelGroup);
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
		case eLIGHT_TYPE::DIR:
			delete pDirLights[i];
			pDirLights.erase(pDirLights.begin() + i);
			break;
		case eLIGHT_TYPE::PNT:
			delete pPointLights[i];
			pPointLights.erase(pPointLights.begin() + i);
			break;
		case eLIGHT_TYPE::SPT:
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
	UINT D3D11::GetModelSize(int i)
	{
		std::vector<Model*>& pModels = m_pModelGroups[i]->GetModels();
		return pModels.size();
	}
	UINT D3D11::GetLightSize(eLIGHT_TYPE eLType)
	{
		UINT size=0;
		switch (eLType)
		{
		case eLIGHT_TYPE::DIR:
			size = m_pLightBuffer->GetDirLightSize();
			break;
		case eLIGHT_TYPE::PNT:
			size = m_pLightBuffer->GetPointLightSize();
			break;
		case eLIGHT_TYPE::SPT:
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
			OutputDebugStringA("D3D11::m_pViewportRTT::CreateTexFailed");
		}
		m_pViewportRTT->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pViewportRTT") - 1, "D3D11::m_pViewportRTT");

		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pSceneRTT);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSceneRTT::CreateTexFailed");
		}
		m_pSceneRTT->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSceneRTT") - 1, "D3D11::m_pSceneRTT");


		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pSSAOBlurDebugRTT);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSSAOBlurDebugRTT::CreateTexFailed");
		}
		m_pSSAOBlurDebugRTT->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSSAOBlurDebugRTT") - 1, "D3D11::m_pSSAOBlurDebugRTT");


		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pBrightRTT);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pBrightRTT::CreateTexFailed");
		}
		m_pBrightRTT->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pBrightRTT") - 1, "D3D11::m_pBrightRTT");


		for (int i = 0; i < 2; ++i)
		{
			hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pPingPongRTT[i]);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::m_pPingPongRTT[i]::CreateTexFailed");
			}
			m_pPingPongRTT[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pPingPongRTT[i]") - 1, "D3D11::m_pPingPongRTT[i]");
		}
		for (int i = 0; i < _GBUF_CNT; ++i)
		{
			hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pGbufferRTT[i]);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::m_pGbufferRTT[i]::CreateTexFailed");
			}
			m_pGbufferRTT[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pGbufferRTT[i]") - 1, "D3D11::m_pGbufferRTT[i]");
		}


		RTTVDesc.Format = RTTDesc.Format;
		RTTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		RTTVDesc.Texture2D.MipSlice = 0;

		hr = m_pDevice->CreateRenderTargetView(m_pViewportRTT, &RTTVDesc, &m_pViewportRTTV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pViewportRTTV::CreateRTVFailed");
		}
		m_pViewportRTTV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pViewportRTTV") - 1, "D3D11::m_pViewportRTTV");


		hr = m_pDevice->CreateRenderTargetView(m_pSceneRTT, &RTTVDesc, &m_pSceneRTTV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSceneRTTV::CreateRTVFailed");
		}
		m_pSceneRTTV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSceneRTTV") - 1, "D3D11::m_pSceneRTTV");


		hr = m_pDevice->CreateRenderTargetView(m_pSSAOBlurDebugRTT, &RTTVDesc, &m_pSSAOBlurDebugRTTV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSSAOBlurDebugRTTV::CreateRTVFailed");
		}
		m_pSSAOBlurDebugRTTV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSSAOBlurDebugRTTV") - 1, "D3D11::m_pSSAOBlurDebugRTTV");

		hr = m_pDevice->CreateRenderTargetView(m_pBrightRTT, &RTTVDesc, &m_pBrightRTTV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pBrightRTTV::CreateRTVFailed");
		}
		m_pBrightRTTV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pBrightRTTV") - 1, "D3D11::m_pBrightRTTV");

	
		for (int i = 0; i < 2; ++i)
		{
			hr = m_pDevice->CreateRenderTargetView(m_pPingPongRTT[i], &RTTVDesc, &m_pPingPongRTTV[i]);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::m_pPingPongRTTV[i]::CreateRTVFailed");
			}
			m_pPingPongRTTV[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pPingPongRTTV[i]") - 1, "D3D11::m_pPingPongRTTV[i]");
		}
		for (int i = 0; i < _GBUF_CNT; ++i)
		{
			hr = m_pDevice->CreateRenderTargetView(m_pGbufferRTT[i], &RTTVDesc, &m_pGbufferRTTV[i]);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::m_pGbufferRTTV[i]::CreateRTVFailed");
			}
			m_pGbufferRTTV[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pGbufferRTTV[i]") - 1, "D3D11::m_pGbufferRTTV[i]");
		}


		SRVDesc.Format = RTTDesc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = m_pDevice->CreateShaderResourceView(m_pViewportRTT, &SRVDesc, &m_pViewportSRV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pViewportSRV[i]::CreateSRVFailed");
		}
		m_pViewportSRV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pViewportSRV") - 1, "D3D11::m_pViewportSRV");


		hr = m_pDevice->CreateShaderResourceView(m_pSceneRTT, &SRVDesc, &m_pSceneSRV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSceneSRV::CreateSRVFailed");
		}
		m_pSceneSRV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSceneSRV") - 1, "D3D11::m_pSceneSRV");


		hr = m_pDevice->CreateShaderResourceView(m_pSSAOBlurDebugRTT, &SRVDesc, &m_pSSAOBlurDebugSRV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSSAOBlurDebug::CreateSRVFailed");
		}
		m_pSSAOBlurDebugSRV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSSAOBlurDebugSRV") - 1, "D3D11::m_pSSAOBlurDebugSRV");


		hr = m_pDevice->CreateShaderResourceView(m_pBrightRTT, &SRVDesc, &m_pBrightSRV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pBrightSRV::CreateSRVFailed");
		}
		m_pBrightSRV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pBrightSRV") - 1, "D3D11::m_pBrightSRV");


		for (int i = 0; i < 2; ++i)
		{
			hr = m_pDevice->CreateShaderResourceView(m_pPingPongRTT[i], &SRVDesc, &m_pPingPongSRV[i]);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::m_pPingPongSRV[i]::CreateSRVFailed");
			}
			m_pPingPongSRV[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pPingPongSRV[i]") - 1, "D3D11::m_pPingPongSRV[i]");
		}
		for (int i = 0; i < _GBUF_CNT; ++i)
		{
			hr = m_pDevice->CreateShaderResourceView(m_pGbufferRTT[i], &SRVDesc, &m_pGbufferSRV[i]);
			if (FAILED(hr))
			{
				OutputDebugStringA("D3D11::m_pGbufferSRV[i]::CreateSRVFailed");
			}
			m_pGbufferSRV[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pGbufferSRV[i]") - 1, "D3D11::m_pGbufferSRV[i]");
		}

		RTTDesc.Format = DXGI_FORMAT_R32_FLOAT;
		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pSSAORTT);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSSAORTT::CreateTexFailed");
		}
		m_pSSAORTT->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSSAORTT") - 1, "D3D11::m_pSSAORTT");

		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pSSAOBlurRTT);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSSAOBlurRTT::CreateTexFailed");
		}
		m_pSSAOBlurRTT->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSSAOBlurRTT") - 1, "D3D11::m_pSSAOBlurRTT");


		RTTVDesc.Format = RTTDesc.Format;
		hr = m_pDevice->CreateRenderTargetView(m_pSSAORTT, &RTTVDesc, &m_pSSAORTTV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSSAORTTV::CreateRTVFailed");
		}
		m_pSSAORTTV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSSAORTTV") - 1, "D3D11::m_pSSAORTTV");


		hr = m_pDevice->CreateRenderTargetView(m_pSSAOBlurRTT, &RTTVDesc, &m_pSSAOBlurRTTV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSSAOBlurRTTV::CreateRTVFailed");
		}
		m_pSSAOBlurRTTV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSSAOBlurRTTV") - 1, "D3D11::m_pSSAOBlurRTTV");


		SRVDesc.Format = RTTVDesc.Format;
		hr = m_pDevice->CreateShaderResourceView(m_pSSAORTT, &SRVDesc, &m_pSSAOSRV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSSAOSRV::CreateSRVFailed");
		}
		m_pSSAOSRV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSSAOSRV") - 1, "D3D11::m_pSSAOSRV");

		hr = m_pDevice->CreateShaderResourceView(m_pSSAOBlurRTT, &SRVDesc, &m_pSSAOBlurSRV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSSAOBlurSRV::CreateSRVVFailed");
		}
		m_pSSAOBlurSRV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSSAOBlurSRV") - 1, "D3D11::m_pSSAOBlurSRV");

		

		RTTDesc.Width = _SHADOWMAP_SIZE;
		RTTDesc.Height = _SHADOWMAP_SIZE;
		RTTDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pBRDFRTT);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pBRDFRTT::CreateTexFailed");
		}
		m_pBRDFRTT->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pBRDFRTT") - 1, "D3D11::m_pBRDFRTT");


		RTTDesc.ArraySize = 6;
		RTTDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		RTTDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pSkyBoxRTT);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSkyBoxRTT::CreateTexFailed");
		}
		m_pSkyBoxRTT->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSkyBoxRTT") - 1, "D3D11::m_pSkyBoxRTT");


		RTTDesc.Width = 32;
		RTTDesc.Height = RTTDesc.Width;
		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pDiffIrradianceRTT);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pDiffIrradianceRTT::CreateTexFailed");
		}
		m_pDiffIrradianceRTT->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pDiffIrradianceRTT") - 1, "D3D11::m_pDiffIrradianceRTT");


		RTTDesc.Width = 128;
		RTTDesc.MipLevels = 5;
		RTTDesc.Height = RTTDesc.Width;
		RTTDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pPrefilterRTT);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pPrefilterRTT::CreateTexFailed");
		}
		m_pPrefilterRTT->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pPrefilterRTT") - 1, "D3D11::m_pPrefilterRTT");


		RTTVDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		hr = m_pDevice->CreateRenderTargetView(m_pBRDFRTT, &RTTVDesc, &m_pBRDFRTTV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pBRDFRTTV::CreateRTVFailed");
		}
		m_pBRDFRTTV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pBRDFRTTV") - 1, "D3D11::m_pBRDFRTTV");


		RTTVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		RTTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		RTTVDesc.Texture2DArray.ArraySize = 6;
		RTTVDesc.Texture2DArray.MipSlice = 0;
		RTTVDesc.Texture2DArray.FirstArraySlice = 0;
		hr = m_pDevice->CreateRenderTargetView(m_pSkyBoxRTT, &RTTVDesc, &m_pSkyBoxRTTV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSkyBoxRTTV::CreateRTVFailed");
		}
		m_pSkyBoxRTTV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSkyBoxRTTV") - 1, "D3D11::m_pSkyBoxRTTV");


		hr = m_pDevice->CreateRenderTargetView(m_pDiffIrradianceRTT, &RTTVDesc, &m_pDiffIrradianceRTTV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pDiffIrradianceRTTV::CreateRTVFailed");
		}
		m_pDiffIrradianceRTTV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pDiffIrradianceRTTV") - 1, "D3D11::m_pDiffIrradianceRTTV");


		hr = m_pDevice->CreateRenderTargetView(m_pPrefilterRTT, &RTTVDesc, &m_pPrefilterRTTV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pPrefilterRTTV::CreateRTVFailed");
		}
		m_pPrefilterRTTV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pPrefilterRTTV") - 1, "D3D11::m_pPrefilterRTTV");



		SRVDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		hr = m_pDevice->CreateShaderResourceView(m_pBRDFRTT, &SRVDesc, &m_pBRDFSRV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pBRDFSRV::CreateSRVFailed");
		}
		m_pBRDFSRV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pBRDFSRV") - 1, "D3D11::m_pBRDFSRV");


		SRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		SRVDesc.TextureCube.MipLevels = 1;
		SRVDesc.TextureCube.MostDetailedMip = 0;
		hr = m_pDevice->CreateShaderResourceView(m_pSkyBoxRTT, &SRVDesc, &m_pSkyBoxSRV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pSkyBoxSRV::CreateSRVFailed");
		}
		m_pSkyBoxSRV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSkyBoxSRV") - 1, "D3D11::m_pSkyBoxSRV");


		hr = m_pDevice->CreateShaderResourceView(m_pDiffIrradianceRTT, &SRVDesc, &m_pDiffIrradianceSRV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pDiffIrradianceSRV::CreateSRVFailed");
		}
		m_pDiffIrradianceSRV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pDiffIrradianceSRV") - 1, "D3D11::m_pDiffIrradianceSRV");


		SRVDesc.TextureCube.MipLevels = 5;
		hr = m_pDevice->CreateShaderResourceView(m_pPrefilterRTT, &SRVDesc, &m_pPrefilterSRV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pPrefilterSRV::CreateSRVFailed");
		}
		m_pPrefilterSRV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pPrefilterSRV") - 1, "D3D11::m_pPrefilterSRV");


		return true;
	}
	bool D3D11::CreateDepthBuffer(int width, int height,
		ID3D11Texture2D** DepthRTT,
		ID3D11DepthStencilView** DepthDSV)
	{
		D3D11_TEXTURE2D_DESC depthBufferDesc;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		HRESULT hr;

		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		depthBufferDesc.Width = width;
		depthBufferDesc.Height = height;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;

		hr = m_pDevice->CreateTexture2D(&depthBufferDesc, nullptr, DepthRTT);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::D3D11::DepthRTT::CreateTexFailed");
		}
		(*DepthRTT)->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::DepthRTT") - 1, "D3D11::DepthRTT");


		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;


		hr = m_pDevice->CreateDepthStencilView(*DepthRTT, &depthStencilViewDesc, DepthDSV);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::D3D11::DepthDSV::CreateDSVFailed");
		}
		(*DepthDSV)->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::DepthDSV") - 1, "D3D11::DepthDSV");

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
			OutputDebugStringA("D3D11::m_pOutlinerSetupDSS::CreateDSSFailed");
		}
		m_pOutlinerSetupDSS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pOutlinerSetupDSS") - 1, "D3D11::m_pOutlinerSetupDSS");


		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;

		hr = m_pDevice->CreateDepthStencilState(&depthStencilDesc, &m_pOutlinerTestDSS);
		if (FAILED(hr))
		{
			OutputDebugStringA("D3D11::m_pOutlinerTestDSS::CreateDSSFailed");
		}
		m_pOutlinerTestDSS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pOutlinerTestDSS") - 1, "D3D11::m_pOutlinerTestDSS");

		return true;
	}

	void D3D11::DrawENTT(bool bGeoPass)
	{	
		float color[4] = { 0.0f, 0.0f,0.0f, 1.0f };
		HRESULT hr;
		XMMATRIX worldMat;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		UINT stride = sizeof(XMFLOAT3);
		UINT offset = 0;
		UINT ENTTCnt = 0;
		UINT ENTTDrawn = 0;
		Plane* pPlanes = m_pFrustum->GetPlanes();
		
		for (int i = 0; i < m_pModelGroups.size(); ++i)
		{	

			std::vector<Model*> pModels = m_pModelGroups[i]->GetModels();
			{
				{
					for (int j = 0; j < pModels.size(); ++j)
					{	
						ENTTCnt++;
						worldMat = pModels[j]->GetTransformMatrix(false);
						AABB* aabb = pModels[j]->GetAABB();
						if (aabb->IsOnFrustum(pPlanes, worldMat))
						{	
							ENTTDrawn++;
							m_pMatBuffer->SetWorldMatrix(&worldMat);
							m_pMatBuffer->Update(m_pContext);
						
					
							bool isSelected = (i == m_selectedModelGroup && j == m_selectedModel);
							if (isSelected)
							{
								m_pContext->OMSetDepthStencilState(m_pOutlinerSetupDSS, 1);
							}
							

							for (int k = 0; k < pModels[j]->GetMatCount(); ++k)
							{
								pModels[j]->UploadBuffers(m_pContext, k, bGeoPass);
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
		m_pFrustum->SetENTTsInFrustum(ENTTDrawn);
		m_pFrustum->SetENTTsInTotal(ENTTCnt);
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

		if (m_pSSAOBlurDebugRTT != nullptr)
		{
			m_pSSAOBlurDebugRTT->Release();
			m_pSSAOBlurDebugRTT = nullptr;
		}

		if (m_pSSAOBlurDebugRTTV != nullptr)
		{
			m_pSSAOBlurDebugRTTV->Release();
			m_pSSAOBlurDebugRTTV = nullptr;
		}

		if (m_pSSAOBlurDebugSRV != nullptr)
		{
			m_pSSAOBlurDebugSRV->Release();
			m_pSSAOBlurDebugSRV = nullptr;
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


		if (m_pSkyBoxRTT != nullptr)
		{
			m_pSkyBoxRTT->Release();
			m_pSkyBoxRTT= nullptr;
		}
		if (m_pSkyBoxRTTV != nullptr)
		{
			m_pSkyBoxRTTV->Release();
			m_pSkyBoxRTTV= nullptr;
		}
		if (m_pSkyBoxSRV != nullptr)
		{
			m_pSkyBoxSRV->Release();
			m_pSkyBoxSRV = nullptr;
		}
		
		
		if (m_pDiffIrradianceRTT != nullptr)
		{
			m_pDiffIrradianceRTT->Release();
			m_pDiffIrradianceRTT = nullptr;
		}
		if (m_pDiffIrradianceRTTV != nullptr)
		{
			m_pDiffIrradianceRTTV->Release();
			m_pDiffIrradianceRTTV = nullptr;
		}
		if (m_pDiffIrradianceSRV != nullptr)
		{
			m_pDiffIrradianceSRV->Release();
			m_pDiffIrradianceSRV = nullptr;
		}



		if (m_pPrefilterRTT != nullptr)
		{
			m_pPrefilterRTT->Release();
			m_pPrefilterRTT = nullptr;
		}
		if (m_pPrefilterRTTV != nullptr)
		{
			m_pPrefilterRTTV->Release();
			m_pPrefilterRTTV = nullptr;
		}
		if (m_pPrefilterSRV != nullptr)
		{
			m_pPrefilterSRV->Release();
			m_pPrefilterSRV = nullptr;
		}


		if (m_pBRDFRTT != nullptr)
		{	
			m_pBRDFRTT->Release();
			m_pBRDFRTT = nullptr;
		}
		if (m_pBRDFRTTV != nullptr)
		{
			m_pBRDFRTTV->Release();
			m_pBRDFRTTV = nullptr;
		}
		if (m_pBRDFSRV != nullptr)
		{
			m_pBRDFSRV->Release();
			m_pBRDFSRV= nullptr;
		}

	}
	void D3D11::DestroyDSBforRTT()
	{
		if (m_pSceneDepthRTT != nullptr)
		{
			m_pSceneDepthRTT->Release();
			m_pSceneDepthRTT = nullptr;
		}

		if (m_pSceneDSV != nullptr)
		{
			m_pSceneDSV->Release();
			m_pSceneDSV = nullptr;
		}
	}
	
}