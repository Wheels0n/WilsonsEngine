#define STB_IMAGE_IMPLEMENTATION
#include "typedef.h"
#include "D3D11.h"
#include "../ImGui/imgui_impl_dx11.h"
#include "stb_image.h"

namespace wilson
{	
	#ifdef _DEBUG

	#include <dxgidebug.h>
	DEFINE_GUID(DXGI_DEBUG_D3D11, 0x4b99317b, 0xac39, 0x4aa6, 0xbb, 0xb, 0xba, 0xa0, 0x47, 0x84, 0x79, 0x8f);

	#pragma comment(lib, "d3d11.lib")
	#pragma comment(lib, "dxguid.lib")

	void D3D11::D3DMemoryLeakCheck()
	{
		HMODULE dxgidebugDLL = GetModuleHandleW(L"dxgidebug.dll");
		decltype(&DXGIGetDebugInterface) GetDebugInterface =
		reinterpret_cast<decltype(&DXGIGetDebugInterface)>(GetProcAddress(dxgidebugDLL, "DXGIGetDebugInterface"));

		IDXGIDebug* pDebug;
		GetDebugInterface(IID_PPV_ARGS(&pDebug));

		OutputDebugStringW(L"!!!D3D 메모리 누수 체크!!!\r\n");
		pDebug->ReportLiveObjects(DXGI_DEBUG_D3D11, DXGI_DEBUG_RLO_DETAIL);
		OutputDebugStringW(L"!!!반환되지 않은 IUnKnown 객체!!!\r\n");

		pDebug->Release();
	}
	#endif // _DEBUG

	D3D11::D3D11(const UINT screenWidth, const UINT screenHeight, const bool bVsync, HWND hWnd, 
		const bool bFullscreen,
		const float fScreenFar, const float fScreenNear)
	{
		m_viewport = {};
		m_prefilterViewport = {};
		m_diffIrradViewport = {};

		m_pCam = nullptr;
		m_pFrustum = nullptr;
		m_pLightCb = nullptr;
		m_pMatricesCb = nullptr;
		m_pShader = nullptr;
		m_pShadowMap = nullptr;

		m_clientWidth = 0;
		m_clientHeight = 0;
		
		m_selectedObject = -1;
		m_selectedMesh = -1;

		m_bVsyncOn = false;
		m_bAabbGridOn=false;
		m_bHeightOn = FALSE;

		m_exposure = 1.0f;
		m_heightScale = 0.0001f;

		{
			HRESULT hr;
			bool result;
			IDXGIFactory* pFactory;
			IDXGIAdapter* pAdapter;
			IDXGIOutput* pAdapterOutput;
			UINT nModes = 0, i = 0, numerator = 0, denominator = 0;
			size_t strLen = 0;
			DXGI_MODE_DESC* pDisplayModeList = {};
			DXGI_ADAPTER_DESC adapterDesc = {};
			int iError = 0;
			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			D3D_FEATURE_LEVEL featureLevel = {};
			ID3D11Texture2D* pBackbuffer = {};
			D3D11_RASTERIZER_DESC rasterDesc = {};
			D3D11_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
			float fFOV = 0.0f, fScreenAspect = 0.0f;
			m_bVsyncOn = bVsync;

			hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pFactory));
			assert(SUCCEEDED(hr));
			pFactory->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::pFactory") - 1, "D3D11::pFactory");

			//Enumerates video cards
			hr = pFactory->EnumAdapters(0, &pAdapter);
			assert(SUCCEEDED(hr));
			pAdapter->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::pAdapter") - 1, "D3D11::pAdapter");

			//Enumerates outputs(ex:monitor)
			hr = pAdapter->EnumOutputs(0, &pAdapterOutput);
			assert(SUCCEEDED(hr));

			//Each monitor has a set of display modes it supports. A display mode refers to the following data in DXGI_MODE_DESC
			hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &nModes, nullptr);
			assert(SUCCEEDED(hr));

			//Fixing a display mode format, we can get a list of all supported display modes an output supports in that format with the following code :
			pDisplayModeList = new DXGI_MODE_DESC[nModes];
			assert(pDisplayModeList!=nullptr);

			hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &nModes, pDisplayModeList);
			assert(SUCCEEDED(hr));

			hr = pAdapter->GetDesc(&adapterDesc);
			assert(SUCCEEDED(hr));


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

			swapChainDesc.BufferCount = _BUFFER_COUNT;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferDesc.Width = screenWidth;
			swapChainDesc.BufferDesc.Height = screenHeight;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;


			if (m_bVsyncOn == true)
			{
				swapChainDesc.BufferDesc.RefreshRate.Numerator = _REFRESH_RATE;
				swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
			}
			else
			{
				swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
				swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
			}

			swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.Windowed = bFullscreen?FALSE: TRUE;
			swapChainDesc.OutputWindow = hWnd;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapChainDesc.Flags = 0;

			featureLevel = D3D_FEATURE_LEVEL_11_0;


			hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1,
				D3D11_SDK_VERSION, &swapChainDesc, m_pSwapChain.GetAddressOf(), m_pDevice.GetAddressOf(), nullptr, m_pContext.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pDevice->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pDevice") - 1, "D3D11::m_pDevice");
			m_pContext->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pContext") - 1, "D3D11::m_pContext");
			m_pSwapChain->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pSwapChain") - 1, "D3D11::m_pSwapChain");




			hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackbuffer));
			assert(SUCCEEDED(hr));
			pBackbuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::pBackbuffer") - 1, "D3D11::pBackbuffer");

			hr = m_pDevice->CreateRenderTargetView(pBackbuffer, nullptr, m_pScreenRtv.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pScreenRtv->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pScreenRtv") - 1, "D3D11::m_pScreenRtv");


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


				hr = m_pDevice->CreateBuffer(&quadBDSC, &quadData, m_pQuadVb.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pQuadVb->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pQuadVb") - 1, "D3D11::m_pQuadVb");


				unsigned long indices[6] = { 0,1,2, 2,3,0 };
				D3D11_SUBRESOURCE_DATA indexData;
				indexData.pSysMem = indices;
				indexData.SysMemPitch = 0;
				indexData.SysMemSlicePitch = 0;

				D3D11_BUFFER_DESC indexBd = { 0, };
				indexBd.Usage = D3D11_USAGE_DEFAULT;
				indexBd.ByteWidth = sizeof(unsigned long) * 6;
				indexBd.BindFlags = D3D11_BIND_INDEX_BUFFER;

				hr = m_pDevice->CreateBuffer(&indexBd, &indexData, m_pQuadIb.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pQuadIb->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pQuadIb") - 1, "D3D11::m_pQuadIb");

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
				UINT nVertex = sizeof(vertices) / sizeof(XMFLOAT3);

				D3D11_SUBRESOURCE_DATA skyBoxVertexData;
				skyBoxVertexData.pSysMem = vertices;
				skyBoxVertexData.SysMemPitch = 0;
				skyBoxVertexData.SysMemSlicePitch = 0;

				D3D11_BUFFER_DESC skyBoxVertexBD;
				skyBoxVertexBD.Usage = D3D11_USAGE_DEFAULT;
				skyBoxVertexBD.ByteWidth = sizeof(XMFLOAT3) * nVertex;
				skyBoxVertexBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				skyBoxVertexBD.CPUAccessFlags = 0;
				skyBoxVertexBD.MiscFlags = 0;
				skyBoxVertexBD.StructureByteStride = 0;

				hr = m_pDevice->CreateBuffer(&skyBoxVertexBD, &skyBoxVertexData, m_pCubeVertices.GetAddressOf());
				assert(SUCCEEDED(hr));
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
				UINT nIndex = sizeof(indices) / sizeof(unsigned long);

				D3D11_SUBRESOURCE_DATA skyBoxIndexData;
				skyBoxIndexData.pSysMem = indices;
				skyBoxIndexData.SysMemPitch = 0;
				skyBoxIndexData.SysMemSlicePitch = 0;

				D3D11_BUFFER_DESC skyBoxIndexBd;
				skyBoxIndexBd.Usage = D3D11_USAGE_DEFAULT;
				skyBoxIndexBd.ByteWidth = sizeof(unsigned long) * nIndex;
				skyBoxIndexBd.BindFlags = D3D11_BIND_INDEX_BUFFER;
				skyBoxIndexBd.CPUAccessFlags = 0;
				skyBoxIndexBd.MiscFlags = 0;
				skyBoxIndexBd.StructureByteStride = 0;

				hr = m_pDevice->CreateBuffer(&skyBoxIndexBd, &skyBoxIndexData, m_pCubeIndices.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pCubeIndices->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pCubeIndices") - 1, "D3D11::m_pCubeIndices");


				D3D11_DEPTH_STENCIL_DESC skyboxDsd;
				ZeroMemory(&skyboxDsd, sizeof(D3D11_DEPTH_STENCIL_DESC));
				skyboxDsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
				hr = m_pDevice->CreateDepthStencilState(&skyboxDsd, m_pSkyBoxDss.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pSkyBoxDss->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pSkyBoxDss") - 1, "D3D11::m_pSkyBoxDss");


				D3D11_RASTERIZER_DESC skyboxRd;
				ZeroMemory(&skyboxRd, sizeof(D3D11_RASTERIZER_DESC));
				skyboxRd.FillMode = D3D11_FILL_SOLID;
				skyboxRd.CullMode = D3D11_CULL_BACK;
				hr = m_pDevice->CreateRasterizerState(&skyboxRd, m_pSkyBoxRs.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pSkyBoxRs->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pSkyBoxRs") - 1, "D3D11::m_pSkyBoxRs");

			}

			//GenAABB Debug 
			{
				D3D11_BUFFER_DESC bds = { 0, };
				bds.Usage = D3D11_USAGE_DYNAMIC;
				bds.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				bds.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bds.ByteWidth = sizeof(DirectX::XMFLOAT3) * 8;
				hr = m_pDevice->CreateBuffer(&bds, 0, m_pAabbVb.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pAabbVb->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pAabbVb") - 1, "D3D11::m_pAabbVb");


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
				hr = m_pDevice->CreateBuffer(&bds, &cubeIndexData, m_pAabbIb.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pAabbIb->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pAabbIb") - 1, "D3D11::m_pAabbIb");
			}

			if (!CreateRtvSrv(m_clientWidth, m_clientHeight))
			{
				OutputDebugStringA("D3D11::CreateRtvSrv()Failed");
			}

			if (!CreateDss())
			{
				OutputDebugStringA("D3D11::CreateDss()Failed");
			}

			if (!CreateDsv(m_clientWidth, m_clientHeight, m_pScreenDepthTex.GetAddressOf(), m_ScreenDsv.GetAddressOf()))
			{
				OutputDebugStringA("D3D11::CreateDsv()onScreenFailed");
			}

			if (!CreateDsv(m_clientWidth, m_clientHeight, m_pSceneDepthTex.GetAddressOf(), m_SceneDsv.GetAddressOf()))
			{
				OutputDebugStringA("D3D11::CreateDsv()onSceneFailed");
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

			hr = m_pDevice->CreateRasterizerState(&rasterDesc, m_pGeoRs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pGeoRs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pGeoRs") - 1, "D3D11::m_pGeoRs");

			rasterDesc.FrontCounterClockwise = false;
			hr = m_pDevice->CreateRasterizerState(&rasterDesc, m_pQuadRs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pQuadRs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pQuadRs") - 1, "D3D11::m_pQuadRs");

			rasterDesc.FrontCounterClockwise = false;
			rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
			hr = m_pDevice->CreateRasterizerState(&rasterDesc, m_pAabbRs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pAabbRs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pAabbRs") - 1, "D3D11::m_pAabbRs");

			m_viewport.Width = static_cast<float>(screenWidth);
			m_viewport.Height = static_cast<float>(screenHeight);
			m_viewport.MinDepth = 0.0f;
			m_viewport.MaxDepth = 1.0f;
			m_viewport.TopLeftX = 0.0f;
			m_viewport.TopLeftY = 0.0f;

			m_diffIrradViewport = m_viewport;
			m_diffIrradViewport.Width = _DIFFIRRAD_WIDTH;
			m_diffIrradViewport.Height =_DIFFIRRAD_HEIGHT;

			m_prefilterViewport = m_viewport;
			m_prefilterViewport.Width = _PREFILTER_WIDTH;
			m_prefilterViewport.Height = _PREFILTER_HEIGHT;

			D3D11_SAMPLER_DESC samplerDesc = {};
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

			hr = m_pDevice->CreateSamplerState(&samplerDesc, m_pWrapSs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pWrapSs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pWrapSs") - 1, "D3D11::m_pWrapSs");

			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			hr = m_pDevice->CreateSamplerState(&samplerDesc, m_pClampSs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pClampSs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pClampSs") - 1, "D3D11::m_pClampSs");

			rtBlendDesc.BlendEnable = TRUE;
			rtBlendDesc.SrcBlend = D3D11_BLEND_ONE;
			rtBlendDesc.DestBlend = D3D11_BLEND_ZERO;
			rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
			rtBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
			rtBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
			rtBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			rtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			D3D11_BLEND_DESC blendDesc = { FALSE, FALSE, rtBlendDesc };
			hr = m_pDevice->CreateBlendState(&blendDesc, m_pGBufferWriteBs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pGBufferWriteBs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pGBufferWriteBs") - 1, "D3D11::m_pGBufferWriteBs");


			rtBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0] = rtBlendDesc;
			hr = m_pDevice->CreateBlendState(&blendDesc, m_pLightingPassBs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pLightingPassBs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pLightingPassBs") - 1, "D3D11::m_pLightingPassBs");


			//Set projectionMatrix, viewMatrix;
			m_pCam = new Camera11(m_pDevice.Get(), screenWidth, screenHeight, fScreenFar, fScreenNear);
			m_pCam->UploadCamPos(m_pContext.Get());
			XMMATRIX* m_projMat = m_pCam->GetProjectionMatrix();
			XMMATRIX* m_viewMat = m_pCam->GetViewMatrix();
			m_pFrustum = new Frustum11(m_pCam);
			m_pMatricesCb = new MatBuffer11(m_pDevice.Get(), m_pContext.Get(), m_viewMat, m_projMat);

			m_pLightCb = new LightBuffer11(m_pDevice.Get());

			m_pShader = new Shader11(m_pDevice.Get(), m_pContext.Get());

			m_pShadowMap = new ShadowMap(m_pDevice.Get(), _SHADOWMAP_SIZE, _SHADOWMAP_SIZE, m_pCam->GetCascadeLevels().size(),
				m_pLightCb->GetDirLightsCapacity(), m_pLightCb->GetCubeLightsCapacity(), m_pLightCb->GetSpotLightsCapacity());

			//Gen User-Defined Class
			{
				D3D11_BUFFER_DESC bufferDesc = { 0, };
				bufferDesc.ByteWidth = sizeof(BOOL) * 4;
				bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
				bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				hr = m_pDevice->CreateBuffer(&bufferDesc, 0, m_pBoolCb.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pBoolCb->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pBoolCb") - 1, "D3D11::m_pBoolCb");

				hr = m_pDevice->CreateBuffer(&bufferDesc, 0, m_pHeightOnOffCb.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pHeightOnOffCb->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pHeightOnOffCb") - 1, "D3D11::m_pHeightOnOffCb");


				bufferDesc.ByteWidth = sizeof(XMVECTOR);
				hr = m_pDevice->CreateBuffer(&bufferDesc, 0, m_pColorCb.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pColorCb->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pColorCb") - 1, "D3D11::m_pColorCb");

				bufferDesc.ByteWidth = sizeof(SamplePoints);
				hr = m_pDevice->CreateBuffer(&bufferDesc, 0, m_pSsaoKernelCb.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pSsaoKernelCb->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pSsaoKernelCb") - 1, "D3D11::m_pSsaoKernelCb");

				bufferDesc.ByteWidth = sizeof(XMFLOAT4);
				hr = m_pDevice->CreateBuffer(&bufferDesc, 0, m_pExposureCb.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pExposureCb->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pExposureCb") - 1, "D3D11::m_pExposureCb");


				hr = m_pDevice->CreateBuffer(&bufferDesc, 0, m_pHeightScaleCb.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pHeightScaleCb->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pHeightScaleCb") - 1, "D3D11::m_pHeightScaleCb");


				bufferDesc.ByteWidth = sizeof(XMMATRIX) * 6;
				hr = m_pDevice->CreateBuffer(&bufferDesc, 0, m_pEquirect2CubeCb.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pEquirect2CubeCb->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pEquirect2CubeCb") - 1, "D3D11::m_pEquirect2CubeCb");

			}

			

			//GenSSAOResources
			{

				//Gen Sample points
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				SamplePoints* pSamplePoints;
				hr = m_pContext->Map(m_pSsaoKernelCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				assert(SUCCEEDED(hr));
				pSamplePoints = reinterpret_cast<SamplePoints*>(mappedResource.pData);

				std::uniform_real_distribution<float> randomFloats(0.0, 1.0f);
				std::default_random_engine gen;
				for (int i = 0; i < _KERNEL_COUNT; ++i)
				{
					XMFLOAT3 sample(
						randomFloats(gen) * 2.0f - 1.0f,
						randomFloats(gen) * 2.0f - 1.0f,
						randomFloats(gen));
					XMVECTOR sampleV = XMLoadFloat3(&sample);
					sampleV = DirectX::XMVector3Normalize(sampleV);
					sampleV = XMVectorScale(sampleV, randomFloats(gen));
					float scale = (float)i / _KERNEL_COUNT;
					scale = 0.1f + (1.0f - 0.1f) * scale * scale;
					sampleV = XMVectorScale(sampleV, scale);

					pSamplePoints->coord[i] = sampleV;
				}
				m_pContext->Unmap(m_pSsaoKernelCb.Get(), 0);


				//Gen noise texture
				for (int i = 0; i < _NOISE_VEC_COUNT; ++i)
				{
					XMFLOAT3 rot(
						randomFloats(gen) * 2.0f - 1.0f,
						randomFloats(gen) * 2.0f - 1.0f,
						0.0f);
					m_rotationVecs.push_back(rot);
				}
				D3D11_TEXTURE2D_DESC texDesc= {};
				texDesc.Width = _NOISE_TEX_SIZE;
				texDesc.Height = _NOISE_TEX_SIZE;
				texDesc.Usage = D3D11_USAGE_DEFAULT;
				texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				texDesc.ArraySize = 1;
				texDesc.MipLevels = 1;
				texDesc.SampleDesc.Count = 1;
				D3D11_SUBRESOURCE_DATA data = { 0, };
				data.pSysMem = &m_rotationVecs[0];
				data.SysMemPitch = texDesc.Width * sizeof(XMFLOAT3);

				hr = m_pDevice->CreateTexture2D(&texDesc, &data, m_pNoiseTex.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pNoiseTex->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pNoiseTex") - 1, "D3D11::m_pNoiseTex");

				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Format = texDesc.Format;
				srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2D.MostDetailedMip = 0;
				hr = m_pDevice->CreateShaderResourceView(m_pNoiseTex.Get(), &srvDesc, m_pNoiseSrv.GetAddressOf());
				assert(SUCCEEDED(hr));
				m_pNoiseSrv->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D11::m_pNoiseSrv") - 1, "D3D11::m_pNoiseSrv");
			}
			//GenPBR Resources	
			{  
				
				bool result = 
					CreateEquirentangularMap(".\\Assets\\Models\\FBX\\Bistro_v5_2\\Bistro_v5_2\\san_giuseppe_bridge_4k.hdr");
				if (result)
				{
					ConvertEquirectagular2Cube();
					CreateDiffuseIrradianceMap();
					CreatePrefileterMap();
					CreateBrdfMap();
				}

			}
			ImGui_ImplDX11_Init(m_pDevice.Get(), m_pContext.Get());
		}
	}

	D3D11::~D3D11()
	{
		ImGui_ImplDX11_Shutdown();

		DestroyRtvSrv();
		DestroySceneDepthTexDsv();
		DestroyBackBuffer();
		DestroyDss();
		DestroyHdr();

		if (m_pLightCb != nullptr)
		{
			delete m_pLightCb;
			m_pLightCb = nullptr;
		}

		if (m_pMatricesCb != nullptr)
		{
			delete m_pMatricesCb;
			m_pMatricesCb = nullptr;
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
		for (int i = 0; i < m_pObjects.size(); ++i)
		{
			if (m_pObjects[i] != nullptr)
			{
				delete m_pObjects[i];
			}
		}
		m_pObjects.clear();
		m_pObjects.shrink_to_fit();

		#ifdef _DEBUG
			D3DMemoryLeakCheck();
		#endif 
		return;
	}

	bool D3D11::CreateEquirentangularMap(const char* pPath)
	{
		HRESULT hr;
		int width, height, nrComponents;
		float* data = stbi_loadf(pPath,
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

			hr = m_pDevice->CreateTexture2D(&texDesc, &subResource, m_pHdrTex.GetAddressOf());
			stbi_image_free(data);
			assert(SUCCEEDED(hr));
			m_pHdrTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pHdrTex") - 1, "D3D11::m_pHdrTex");


			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = texDesc.Format;
			srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			hr = m_pDevice->CreateShaderResourceView(m_pHdrTex.Get(), &srvDesc, m_pHdrSrv.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pHdrSrv->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pHdrSrv") - 1, "D3D11::m_pHdrSrv");
			return true;
		}
		else
		{
			return false;
		}
	}
	void D3D11::ConvertEquirectagular2Cube()
	{
		//Covenrt EquirectangularMap to CubeMap
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedSubResource;
		hr = m_pContext->Map(m_pEquirect2CubeCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource);
		assert(SUCCEEDED(hr));
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
		XMMATRIX* pMatrix = reinterpret_cast<XMMATRIX*>(mappedSubResource.pData);
		for (int i = 0; i < 6; ++i)
		{
			pMatrix[i] = XMMatrixMultiplyTranspose(capView[i], capProj);

		}
		m_pContext->Unmap(m_pEquirect2CubeCb.Get(), 0);


		stride = sizeof(XMFLOAT3);
		m_pShader->SetPosOnlyInputLayout(m_pContext.Get());
		m_pShader->SetEquirect2CubeShader(m_pContext.Get());
		m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pContext->IASetVertexBuffers(0, 1, &m_pCubeVertices, &stride, &offset);
		m_pContext->IASetIndexBuffer(m_pCubeIndices.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_pContext->GSSetConstantBuffers(0, 1, m_pEquirect2CubeCb.GetAddressOf());
		m_pContext->PSSetShaderResources(0, 1, m_pHdrSrv.GetAddressOf());
		m_pContext->PSSetSamplers(0, 1, m_pWrapSs.GetAddressOf());
		m_pContext->RSSetViewports(1, m_pShadowMap->GetViewport());
		m_pContext->OMSetDepthStencilState(0, 0);
		m_pContext->OMSetRenderTargets(1, m_pSkyBoxRtv.GetAddressOf(), nullptr);
		m_pContext->DrawIndexed(_CUBE_IDX_COUNT, 0, 0);
	}
	void D3D11::CreateDiffuseIrradianceMap()
	{
		//Gen Diffuse IrradianceMap
		m_pShader->SetDiffuseIrradianceShader(m_pContext.Get());
		m_pContext->OMSetRenderTargets(1, m_pDiffIrradianceRtv.GetAddressOf(), nullptr);
		m_pContext->PSSetShaderResources(0, 1, m_pSkyBoxSrv.GetAddressOf());
		m_pContext->PSSetSamplers(0, 1, m_pClampSs.GetAddressOf());
		m_pContext->RSSetViewports(1, &m_diffIrradViewport);
		m_pContext->DrawIndexed(36, 0, 0);
	}
	void D3D11::CreatePrefileterMap()
	{
		//Gen PrefileterMap;
		m_pShader->SetPrefilterShader(m_pContext.Get());
		m_pContext->PSSetSamplers(0, 1, m_pWrapSs.GetAddressOf());
		m_pContext->RSSetViewports(1, &m_prefilterViewport);
		m_pContext->OMSetRenderTargets(1, m_pPrefilterRtv.GetAddressOf(), nullptr);
		m_pContext->DrawIndexed(36, 0, 0);
		m_pContext->GenerateMips(m_pPrefilterSrv.Get());
	}
	void D3D11::CreateBrdfMap()
	{	
		UINT stride;
		UINT offset=0;

		//Gen BRDFMap
		m_pShader->SetTexInputlayout(m_pContext.Get());
		m_pShader->SetBrdfShader(m_pContext.Get());
		stride = sizeof(QUAD);
		m_pContext->IASetVertexBuffers(0, 1, m_pQuadVb.GetAddressOf(), &stride, &offset);
		m_pContext->IASetIndexBuffer(m_pQuadIb.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_pContext->RSSetViewports(1, m_pShadowMap->GetViewport());
		m_pContext->PSSetSamplers(0, 1, m_pClampSs.GetAddressOf());
		m_pContext->OMSetRenderTargets(1, m_pBrdfRtv.GetAddressOf(), nullptr);
		m_pContext->DrawIndexed(6, 0, 0);
	}
	void D3D11::ResizeBackBuffer(const UINT newWidth, const UINT newHeight)
	{	
		m_clientWidth = newWidth;
		m_clientHeight = newHeight;

		m_pContext->OMSetRenderTargets(0, nullptr, nullptr);
		DestroyRtvSrv();
		DestroySceneDepthTexDsv();
		DestroyHdr();
		DestroyBackBuffer();

		m_pSwapChain->ResizeBuffers(1, m_clientWidth, m_clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);


		HRESULT hr;
		ID3D11Texture2D* pBackbuffer;
		hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackbuffer));
		assert(SUCCEEDED(hr));
		pBackbuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::pBackbuffer") - 1, "D3D11::pBackbuffer");
		hr = m_pDevice->CreateRenderTargetView(pBackbuffer, nullptr, m_pScreenRtv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pScreenRtv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pScreenRtv") - 1, "D3D11::m_pScreenRtv");
		pBackbuffer->Release();
		
		
		if (!CreateRtvSrv(m_clientWidth, m_clientHeight))
		{
			OutputDebugStringA("D3D11::CreateRtvSrv()Failed");
		}

		if (!CreateDsv(m_clientWidth, m_clientHeight, m_pScreenDepthTex.GetAddressOf(), m_ScreenDsv.GetAddressOf()))
		{
			OutputDebugStringA("D3D11::CreateDsv()onScreenFailed");
		}

		if (!CreateDsv(m_clientWidth, m_clientHeight, m_pSceneDepthTex.GetAddressOf(), m_SceneDsv.GetAddressOf()))
		{
			OutputDebugStringA("D3D11::CreateDsv()onSceneFailed");
		}
		m_viewport.Width = static_cast<float>(m_clientWidth);
		m_viewport.Height = static_cast<float>(m_clientHeight);

		
		bool result =
			CreateEquirentangularMap(".\\Assets\\Models\\FBX\\Bistro_v5_2\\Bistro_v5_2\\san_giuseppe_bridge_4k.hdr");
		if (result)
		{
			ConvertEquirectagular2Cube();
			CreateDiffuseIrradianceMap();
			CreatePrefileterMap();
			CreateBrdfMap();
		}
	}

	void D3D11::UpdateScene()
	{
		//clear views
		HRESULT hr;
		float color[4] = { 0.0f, 0.0f,0.0f, 1.0f };
		UINT texCnt = static_cast<int>(eGbuf::cnt) -2;
		UINT stride;
		UINT offset = 0;
		UINT drawed = 0;
		bool bGeoPass = false;
		bool bSpotShadowPass = false;


		ID3D11RenderTargetView* nullRtvs[static_cast<UINT>(eGbuf::cnt)] = { nullptr, };
		ID3D11RenderTargetView* pSsaoRtvs[2] = {m_pSsaoBlurRtv.Get(), m_pSsaoBlurDebugRtv.Get() };

		m_pContext->ClearRenderTargetView(m_pScreenRtv.Get(), color);
		m_pContext->ClearRenderTargetView(m_pViewportRtv.Get(), color);
		m_pContext->ClearRenderTargetView(m_pSceneRtv.Get(), color);
		m_pContext->ClearRenderTargetView(m_pSsaoRtv.Get(), color);
		m_pContext->ClearRenderTargetView(m_pSsaoBlurRtv.Get(), color);
		m_pContext->ClearRenderTargetView(m_pSsaoBlurDebugRtv.Get(), color);
		for (int i = 0; i < static_cast<UINT>(eGbuf::cnt); ++i)
		{
			m_pContext->ClearRenderTargetView(m_pGbufferRtvs[i].Get(), color);
		}

		m_pContext->ClearDepthStencilView(m_ScreenDsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_pContext->ClearDepthStencilView(m_SceneDsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_pContext->PSSetSamplers(0, 1, m_pWrapSs.GetAddressOf());
		m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pCam->Update();
		//Update Light11
		std::vector<DirectionalLight11*>& dirLights = m_pLightCb->GetDirLights();
		std::vector<CubeLight11*>& CubeLights = m_pLightCb->GetCubeLights();
		std::vector<SpotLight11*>& spotLights = m_pLightCb->GetSpotLights();
		//Draw ShadowMap
		{	
			m_pContext->PSSetShaderResources(0, 4 +dirLights.capacity()+ CubeLights.capacity()+ spotLights.capacity(), m_pLightCb->GetNullSRVs());
			m_pContext->RSSetViewports(1, m_pShadowMap->GetViewport());
			m_pContext->RSSetState(m_pQuadRs.Get());
			m_pContext->OMSetDepthStencilState(0, 0);
			
		
			m_pShader->SetPosOnlyInputLayout(m_pContext.Get());
			m_pShader->SetCascadeDirShadowShader(m_pContext.Get());
			for (int i = 0; i < dirLights.size(); ++i)
			{	
				dirLights[i]->UpdateLightSpaceMatrices();
				dirLights[i]->UploadShadowMatrices(m_pContext.Get());
				m_pShadowMap->BindDirDsv(m_pContext.Get(),i);
				DrawObject(bGeoPass, bSpotShadowPass);
			}

			m_pShader->SetTexInputlayout(m_pContext.Get());
			m_pShader->SetSpotShadowShader(m_pContext.Get());
			bSpotShadowPass = true;
			for (int i = 0; i < spotLights.size(); ++i)
			{
				m_pMatricesCb->SetLightSpaceMatrix(spotLights[i]->GetLightSpaceMat());
				m_pShadowMap->BindSpotDsv(m_pContext.Get(), i);
				DrawObject(bGeoPass, bSpotShadowPass);
			}
			bSpotShadowPass = false;

			m_pContext->RSSetState(m_pSkyBoxRs.Get());
			m_pShader->SetCubeShadowShader(m_pContext.Get());
			for (int i = 0; i < CubeLights.size(); ++i)
			{
				m_pShadowMap->BindCubeDsv(m_pContext.Get(), i);
				CubeLights[i]->UploadShadowMatrices(m_pContext.Get());
				CubeLights[i]->UploadLightPos(m_pContext.Get());
				DrawObject(bGeoPass, bSpotShadowPass);
			}
			
		}
		//Draw EnvMap
		stride = sizeof(XMFLOAT3);
		m_pMatricesCb->SetWorldMatrix(&m_idMat);	
		m_pMatricesCb->SetViewMatrix(m_pCam->GetViewMatrix());
		m_pMatricesCb->SetProjMatrix(m_pCam->GetProjectionMatrix());
		m_pMatricesCb->UploadMatBuffer(m_pContext.Get(), bSpotShadowPass);
		m_pShader->SetSkyBoxShader(m_pContext.Get());
		m_pContext->IASetVertexBuffers(0, 1, m_pCubeVertices.GetAddressOf(), &stride, &offset);
		m_pContext->IASetIndexBuffer(m_pCubeIndices.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_pContext->PSSetShaderResources(0, 1, m_pSkyBoxSrv.GetAddressOf());
		m_pContext->PSSetShaderResources(1, 1, m_pHdrSrv.GetAddressOf());
		m_pContext->RSSetViewports(1, &m_viewport);
		m_pContext->OMSetDepthStencilState(m_pSkyBoxDss.Get(), 0);
		m_pContext->OMSetRenderTargets(1, m_pGbufferRtvs[2].GetAddressOf(), m_SceneDsv.Get());
		m_pContext->DrawIndexed(_CUBE_IDX_COUNT, 0, 0);
	
		//Deferred Shading First Pass
		if (!m_pObjects.empty())
		{	
			m_pCam->UploadCamPos(m_pContext.Get());
			m_pShader->SetDeferredGeoLayout(m_pContext.Get());
			m_pShader->SetPbrDeferredGeoShader(m_pContext.Get());
			m_pContext->RSSetState(m_pQuadRs.Get());
			m_pContext->OMSetDepthStencilState(0, 0);
			m_pContext->OMSetBlendState(m_pGBufferWriteBs.Get(), color, 0xffffffff);
			m_pContext->OMSetRenderTargets(static_cast<UINT>(eGbuf::cnt), m_pGbufferRtvs[0].GetAddressOf(), m_SceneDsv.Get());


			//Upload HeightScale and bHeightOnOff
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				XMFLOAT4* pHeightScale;
				m_pContext->Map(m_pHeightScaleCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				pHeightScale = reinterpret_cast<XMFLOAT4*>(mappedResource.pData);
				pHeightScale->x = m_heightScale;
				m_pContext->Unmap(m_pHeightScaleCb.Get(), 0);
				m_pContext->PSSetConstantBuffers(3, 1, m_pHeightScaleCb.GetAddressOf());

				BOOL* pbHeightOn;
				m_pContext->Map(m_pHeightOnOffCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				pbHeightOn = reinterpret_cast<BOOL*>(mappedResource.pData);
				*pbHeightOn = m_bHeightOn;
				m_pContext->Unmap(m_pHeightOnOffCb.Get(), 0);
				m_pContext->PSSetConstantBuffers(4, 1, m_pHeightOnOffCb.GetAddressOf());

			}
			DrawObject(!bGeoPass, bSpotShadowPass);
			

			//SSAO Pass
			stride = sizeof(QUAD);
			m_pContext->IASetVertexBuffers(0, 1, &m_pQuadVb, &stride, &offset);
			m_pContext->IASetIndexBuffer(m_pQuadIb.Get(), DXGI_FORMAT_R32_UINT, 0);

			m_pShader->SetTexInputlayout(m_pContext.Get());
			m_pShader->SetSsaoShader(m_pContext.Get());

			m_pContext->PSSetConstantBuffers(0, 1, m_pSsaoKernelCb.GetAddressOf());
			m_pMatricesCb->UploadProjMat(m_pContext.Get());
			m_pContext->OMSetRenderTargets(1, m_pSsaoRtv.GetAddressOf(), nullptr);
			m_pContext->PSSetShaderResources(0, 2, m_ppGbufferSrvs[static_cast<UINT>(eGbuf::cnt) -3].GetAddressOf());
			m_pContext->PSSetShaderResources(2, 1, m_pNoiseSrv.GetAddressOf());
			m_pContext->PSSetSamplers(0, 1, m_pWrapSs.GetAddressOf());
			m_pContext->PSSetSamplers(1, 1, m_pClampSs.GetAddressOf());
			m_pContext->DrawIndexed(6, 0, 0);
			//Blur SSAOTex
			m_pShader->SetSsaoBlurShader(m_pContext.Get());
			m_pContext->OMSetRenderTargets(2, pSsaoRtvs, nullptr);
			m_pContext->PSSetShaderResources(0, 1, m_pSsaoSrv.GetAddressOf());
			m_pContext->DrawIndexed(6, 0, 0);

			//Lighting Pass
			m_pShader->SetTexInputlayout(m_pContext.Get());
			m_pShader->SetPbrDeferredLightingShader(m_pContext.Get());
			m_pLightCb->UpdateDirLightMatrices(m_pContext.Get());
			m_pLightCb->UpdateSpotLightMatrices(m_pContext.Get());
			m_pLightCb->UpdateLightBuffer(m_pContext.Get());
			m_pCam->UploadCamPos(m_pContext.Get());
			m_pCam->UploadCascadeLevels(m_pContext.Get());
	
			m_pContext->OMSetRenderTargets(1, m_pSceneRtv.GetAddressOf(), nullptr);
			m_pContext->OMSetBlendState(m_pLightingPassBs.Get(), color, 0xffffffff);
			m_pContext->PSSetShaderResources(0, static_cast<UINT>(eGbuf::cnt) -2, m_ppGbufferSrvs[0].GetAddressOf());
			m_pContext->PSSetShaderResources(texCnt++, 1, m_pSsaoBlurSrv.GetAddressOf());
			m_pContext->PSSetShaderResources(texCnt++, 1, m_pDiffIrradianceSrv.GetAddressOf());
			m_pContext->PSSetShaderResources(texCnt++, 1, m_pPrefilterSrv.GetAddressOf());
			m_pContext->PSSetShaderResources(texCnt++, 1, m_pBrdfSrv.GetAddressOf());
			
			m_pContext->PSSetShaderResources(texCnt, dirLights.capacity(), m_pShadowMap->GetDirSrv());
			m_pContext->PSSetShaderResources(texCnt +dirLights.capacity(), spotLights.capacity(), m_pShadowMap->GetSpotSrv());
			m_pContext->PSSetShaderResources(texCnt +dirLights.capacity() + spotLights.capacity(), CubeLights.capacity(), m_pShadowMap->GetCubeSrv());
	

			m_pContext->PSSetShaderResources(texCnt++, 1, m_pShadowMap->GetDirSrv());
			m_pContext->PSSetShaderResources(texCnt++, 1, m_pShadowMap->GetSpotSrv());
			m_pContext->PSSetShaderResources(texCnt, 1, m_pShadowMap->GetCubeSrv());
			m_pContext->PSSetSamplers(1, 1, m_pShadowMap->GetCubeShadowSampler());
			m_pContext->PSSetSamplers(2, 1, m_pShadowMap->GetDirShadowSampler());
			m_pContext->DrawIndexed(6, 0, 0);
			
		}
		else
		{	
			if (m_pFrustum->GetSubMeshesInScene())
			{
				m_pFrustum->SetSubMeshesInFrustum(0);
				m_pFrustum->SetSubMeshesInScene(0);
			}
			
		}
		
		//Draw picked Mesh11's Outline
		m_pShader->SetTexInputlayout(m_pContext.Get());
		m_pShader->SetOutlinerTestShader(m_pContext.Get());
		m_pContext->OMSetDepthStencilState(m_pOutlinerTestDss.Get(), 1);
		m_pContext->OMSetRenderTargets(1, m_pSceneRtv.GetAddressOf(), m_SceneDsv.Get());
		if (m_selectedObject != _UNSELECT)
		{
			std::vector<Mesh11*> pMeshes = m_pObjects[m_selectedObject]->GetMeshes();
			XMMATRIX worldMat = pMeshes[m_selectedMesh]->GetTransformMatrix(true);
			m_pMatricesCb->SetWorldMatrix(&worldMat);
			m_pMatricesCb->UploadMatBuffer(m_pContext.Get(), bSpotShadowPass);

			for (int k = 0; k < pMeshes[m_selectedMesh]->GetNumMaterial(); ++k)
			{
				pMeshes[m_selectedMesh]->UploadBuffers(m_pContext.Get(), k, bGeoPass);
				m_pContext->DrawIndexed(pMeshes[m_selectedMesh]->GetNumIndex(k), 0, 0);
			}
		}
		//Submit Result
		stride = sizeof(QUAD);
		m_pContext->IASetVertexBuffers(0, 1, m_pQuadVb.GetAddressOf(), &stride, &offset);
		m_pContext->IASetIndexBuffer(m_pQuadIb.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_pShader->SetTexInputlayout(m_pContext.Get());
		m_pShader->SetPostProcessShader(m_pContext.Get());
		m_pContext->RSSetState(m_pQuadRs.Get());
		m_pContext->OMSetRenderTargets(0, nullptr, nullptr);
		{	
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			XMFLOAT4* pExposure;
			m_pContext->Map(m_pExposureCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			pExposure = reinterpret_cast<XMFLOAT4*>(mappedResource.pData);
			pExposure->x = m_exposure;
			m_pContext->Unmap(m_pExposureCb.Get(), 0);
			m_pContext->PSSetConstantBuffers(0, 1, m_pExposureCb.GetAddressOf());
		}
		
		m_pContext->PSSetShaderResources(0, 1, m_pObjects.empty()||!m_pFrustum->GetSubMeshesInFrustum() ? m_ppGbufferSrvs[2].GetAddressOf() : m_pSceneSrv.GetAddressOf());
		m_pContext->OMSetRenderTargets(1, m_pViewportRtv.GetAddressOf(), nullptr);
		m_pContext->OMSetDepthStencilState(0, 0);
		m_pContext->DrawIndexed(6, 0, 0);
		//DrawUI
		m_pContext->OMSetRenderTargets(1, m_pScreenRtv.GetAddressOf(), m_ScreenDsv.Get());
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


		if (m_bVsyncOn)
		{
			m_pSwapChain->Present(1, 0);
		}
		else
		{
			m_pSwapChain->Present(0, 0);
		}
	}

	void D3D11::AddLight(Light11* const pLight)
	{	
		switch (pLight->GetType())
		{
		case eLIGHT_TYPE::DIR:
			m_pLightCb->PushDirLight(reinterpret_cast<DirectionalLight11*>(pLight));
			break;
		case eLIGHT_TYPE::CUBE:
			m_pLightCb->PushCubeLight(reinterpret_cast<CubeLight11*>(pLight));
			break;
		case eLIGHT_TYPE::SPT:
			m_pLightCb->PushSpotLight(reinterpret_cast<SpotLight11*>(pLight));
		}
	}
	void D3D11::AddObject(Object11* const pObject)
	{
		m_pObjects.push_back(pObject);
	}
	void D3D11::RemoveObject(const UINT i)
	{
		delete m_pObjects[i];
		m_pObjects.erase(m_pObjects.begin() + i);
	}
	void D3D11::RemoveLight(const UINT i, Light11* const pLight)
	{	
		std::vector<DirectionalLight11*>& pDirLights = m_pLightCb->GetDirLights();;
		std::vector<CubeLight11*>& pCubeLights= m_pLightCb->GetCubeLights();
		std::vector<SpotLight11*>& pSpotLights = m_pLightCb->GetSpotLights();
		switch (pLight->GetType())
		{
		case eLIGHT_TYPE::DIR:
			delete pDirLights[i];
			pDirLights.erase(pDirLights.begin() + i);
			break;
		case eLIGHT_TYPE::CUBE:
			delete pCubeLights[i];
			pCubeLights.erase(pCubeLights.begin() + i);
			break;
		case eLIGHT_TYPE::SPT:
			delete pSpotLights[i];
			pSpotLights.erase(pSpotLights.begin() + i);
			break;
		}
	}
	void D3D11::RemoveMesh(const UINT objectIdx, const UINT meshIdx)
	{
		std::vector<Mesh11*>& pMeshes = m_pObjects[objectIdx]->GetMeshes();
		delete pMeshes[meshIdx];
		pMeshes.erase(pMeshes.begin() + meshIdx);
		
	}
	UINT D3D11::GetNumMesh(const UINT i)
	{
		std::vector<Mesh11*>& pModels = m_pObjects[i]->GetMeshes();
		return pModels.size();
	}
	UINT D3D11::GetNumLight(const eLIGHT_TYPE eLType)
	{
		UINT size=0;
		switch (eLType)
		{
		case eLIGHT_TYPE::DIR:
			size = m_pLightCb->GetDirLightsSize();
			break;
		case eLIGHT_TYPE::CUBE:
			size = m_pLightCb->GetCubeLightsSize();
			break;
		case eLIGHT_TYPE::SPT:
			size = m_pLightCb->GetSpotLightsSize();
		}
		return size;
	}

	bool D3D11::CreateRtvSrv(const UINT width, const UINT height)
	{
		DestroyRtvSrv();

		D3D11_TEXTURE2D_DESC texd;
		D3D11_RENDER_TARGET_VIEW_DESC rtvd;
		D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
		HRESULT hr;

		ZeroMemory(&texd, sizeof(texd));
		texd.Width = width;
		texd.Height = height;
		texd.MipLevels = 1;
		texd.ArraySize = 1;
		texd.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texd.SampleDesc.Count = 1;
		texd.SampleDesc.Quality = 0;
		texd.Usage = D3D11_USAGE_DEFAULT;
		texd.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texd.CPUAccessFlags = 0;
		texd.MiscFlags = 0;

		hr = m_pDevice->CreateTexture2D(&texd, nullptr, m_pViewportTex.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pViewportTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pViewportTex") - 1, "D3D11::m_pViewportTex");

		hr = m_pDevice->CreateTexture2D(&texd, nullptr, m_pSceneTex.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSceneTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSceneTex") - 1, "D3D11::m_pSceneTex");


		hr = m_pDevice->CreateTexture2D(&texd, nullptr, m_pSsaoBlurDebugTex.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSsaoBlurDebugTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSsaoBlurDebugTex") - 1, "D3D11::m_pSsaoBlurDebugTex");


		hr = m_pDevice->CreateTexture2D(&texd, nullptr, m_pBrightTex.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pBrightTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pBrightTex") - 1, "D3D11::m_pBrightTex");


		for (int i = 0; i < 2; ++i)
		{
			hr = m_pDevice->CreateTexture2D(&texd, nullptr, m_pPingPongTex[i].GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pPingPongTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pPingPongTex[objectIdx]") - 1, "D3D11::m_pPingPongTex[objectIdx]");
		}
		for (int i = 0; i < static_cast<int>(eGbuf::cnt); ++i)
		{
			hr = m_pDevice->CreateTexture2D(&texd, nullptr, m_pGbufferTex[i].GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pGbufferTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pGbufferTex[objectIdx]") - 1, "D3D11::m_pGbufferTex[objectIdx]");
		}


		rtvd.Format = texd.Format;
		rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvd.Texture2D.MipSlice = 0;

		hr = m_pDevice->CreateRenderTargetView(m_pViewportTex.Get(), &rtvd, m_pViewportRtv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pViewportRtv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pViewportRtv") - 1, "D3D11::m_pViewportRtv");


		hr = m_pDevice->CreateRenderTargetView(m_pSceneTex.Get(), &rtvd, m_pSceneRtv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSceneRtv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSceneRtv") - 1, "D3D11::m_pSceneRtv");


		hr = m_pDevice->CreateRenderTargetView(m_pSsaoBlurDebugTex.Get(), &rtvd, m_pSsaoBlurDebugRtv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSsaoBlurDebugRtv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSsaoBlurDebugRtv") - 1, "D3D11::m_pSsaoBlurDebugRtv");

		hr = m_pDevice->CreateRenderTargetView(m_pBrightTex.Get(), &rtvd, m_pBrightRtv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pBrightRtv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pBrightRtv") - 1, "D3D11::m_pBrightRtv");

	
		for (int i = 0; i < 2; ++i)
		{
			hr = m_pDevice->CreateRenderTargetView(m_pPingPongTex[i].Get(), &rtvd, m_pPingPongRtvs[i].GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pPingPongRtvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pPingPongRtvs[objectIdx]") - 1, "D3D11::m_pPingPongRtvs[objectIdx]");
		}
		for (int i = 0; i < static_cast<int>(eGbuf::cnt); ++i)
		{
			hr = m_pDevice->CreateRenderTargetView(m_pGbufferTex[i].Get(), &rtvd, m_pGbufferRtvs[i].GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pGbufferRtvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pGbufferRtvs[objectIdx]") - 1, "D3D11::m_pGbufferRtvs[objectIdx]");
		}


		srvd.Format = texd.Format;
		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvd.Texture2D.MostDetailedMip = 0;
		srvd.Texture2D.MipLevels = 1;

		hr = m_pDevice->CreateShaderResourceView(m_pViewportTex.Get(), &srvd, m_pViewportSrv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pViewportSrv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pViewportSrv") - 1, "D3D11::m_pViewportSrv");


		hr = m_pDevice->CreateShaderResourceView(m_pSceneTex.Get(), &srvd, m_pSceneSrv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSceneSrv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSceneSrv") - 1, "D3D11::m_pSceneSrv");


		hr = m_pDevice->CreateShaderResourceView(m_pSsaoBlurDebugTex.Get(), &srvd, m_pSsaoBlurDebugSrv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSsaoBlurDebugSrv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSsaoBlurDebugSrv") - 1, "D3D11::m_pSsaoBlurDebugSrv");


		hr = m_pDevice->CreateShaderResourceView(m_pBrightTex.Get(), &srvd, m_pBrightSrv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pBrightSrv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pBrightSrv") - 1, "D3D11::m_pBrightSrv");


		for (int i = 0; i < 2; ++i)
		{
			hr = m_pDevice->CreateShaderResourceView(m_pPingPongTex[i].Get(), &srvd, m_pPingPongSrvs[i].GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pPingPongSrvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_pPingPongSrvs[objectIdx]") - 1, "D3D11::m_pPingPongSrvs[objectIdx]");
		}
		for (int i = 0; i < static_cast<int>(eGbuf::cnt); ++i)
		{
			hr = m_pDevice->CreateShaderResourceView(m_pGbufferTex[i].Get(), &srvd, m_ppGbufferSrvs[i].GetAddressOf());
			assert(SUCCEEDED(hr));
			m_ppGbufferSrvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D11::m_ppGbufferSrvs[objectIdx]") - 1, "D3D11::m_ppGbufferSrvs[objectIdx]");
		}

		texd.Format = DXGI_FORMAT_R32_FLOAT;
		hr = m_pDevice->CreateTexture2D(&texd, nullptr, m_pSsaoTex.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSsaoTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSsaoTex") - 1, "D3D11::m_pSsaoTex");

		hr = m_pDevice->CreateTexture2D(&texd, nullptr, m_pSsaoBlurTex.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSsaoBlurTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSsaoBlurTex") - 1, "D3D11::m_pSsaoBlurTex");


		rtvd.Format = texd.Format;
		hr = m_pDevice->CreateRenderTargetView(m_pSsaoTex.Get(), &rtvd, m_pSsaoRtv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSsaoRtv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSsaoRtv") - 1, "D3D11::m_pSsaoRtv");


		hr = m_pDevice->CreateRenderTargetView(m_pSsaoBlurTex.Get(), &rtvd, m_pSsaoBlurRtv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSsaoBlurRtv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSsaoBlurRtv") - 1, "D3D11::m_pSsaoBlurRtv");


		srvd.Format = rtvd.Format;
		hr = m_pDevice->CreateShaderResourceView(m_pSsaoTex.Get(), &srvd, m_pSsaoSrv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSsaoSrv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSsaoSrv") - 1, "D3D11::m_pSsaoSrv");

		hr = m_pDevice->CreateShaderResourceView(m_pSsaoBlurTex.Get(), &srvd, m_pSsaoBlurSrv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSsaoBlurSrv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSsaoBlurSrv") - 1, "D3D11::m_pSsaoBlurSrv");

		

		texd.Width = _SHADOWMAP_SIZE;
		texd.Height = _SHADOWMAP_SIZE;
		texd.Format = DXGI_FORMAT_R32G32_FLOAT;
		hr = m_pDevice->CreateTexture2D(&texd, nullptr, m_pBrdfTex.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pBrdfTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pBrdfTex") - 1, "D3D11::m_pBrdfTex");


		texd.ArraySize = 6;
		texd.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texd.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		hr = m_pDevice->CreateTexture2D(&texd, nullptr, m_pSkyBoxTex.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSkyBoxTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSkyBoxTex") - 1, "D3D11::m_pSkyBoxTex");


		texd.Width = _DIFFIRRAD_WIDTH;
		texd.Height = texd.Width;
		hr = m_pDevice->CreateTexture2D(&texd, nullptr, m_pDiffIrradianceTex.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pDiffIrradianceTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pDiffIrradianceTex") - 1, "D3D11::m_pDiffIrradianceTex");


		texd.Width = _PREFILTER_WIDTH;
		texd.MipLevels = _PREFILTER_MIP_LEVELS;
		texd.Height = texd.Width;
		texd.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		hr = m_pDevice->CreateTexture2D(&texd, nullptr, m_pPrefilterTex.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pPrefilterTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pPrefilterTex") - 1, "D3D11::m_pPrefilterTex");


		rtvd.Format = DXGI_FORMAT_R32G32_FLOAT;
		hr = m_pDevice->CreateRenderTargetView(m_pBrdfTex.Get(), &rtvd, m_pBrdfRtv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pBrdfRtv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pBrdfRtv") - 1, "D3D11::m_pBrdfRtv");


		rtvd.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvd.Texture2DArray.ArraySize = 6;
		rtvd.Texture2DArray.MipSlice = 0;
		rtvd.Texture2DArray.FirstArraySlice = 0;
		hr = m_pDevice->CreateRenderTargetView(m_pSkyBoxTex.Get(), &rtvd, m_pSkyBoxRtv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSkyBoxRtv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSkyBoxRtv") - 1, "D3D11::m_pSkyBoxRtv");


		hr = m_pDevice->CreateRenderTargetView(m_pDiffIrradianceTex.Get(), &rtvd, m_pDiffIrradianceRtv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pDiffIrradianceRtv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pDiffIrradianceRtv") - 1, "D3D11::m_pDiffIrradianceRtv");


		hr = m_pDevice->CreateRenderTargetView(m_pPrefilterTex.Get(), &rtvd, m_pPrefilterRtv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pPrefilterRtv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pPrefilterRtv") - 1, "D3D11::m_pPrefilterRtv");



		srvd.Format = DXGI_FORMAT_R32G32_FLOAT;
		hr = m_pDevice->CreateShaderResourceView(m_pBrdfTex.Get(), &srvd, m_pBrdfSrv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pBrdfSrv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pBrdfSrv") - 1, "D3D11::m_pBrdfSrv");


		srvd.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvd.TextureCube.MipLevels = 1;
		srvd.TextureCube.MostDetailedMip = 0;
		hr = m_pDevice->CreateShaderResourceView(m_pSkyBoxTex.Get(), &srvd, m_pSkyBoxSrv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSkyBoxSrv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pSkyBoxSrv") - 1, "D3D11::m_pSkyBoxSrv");


		hr = m_pDevice->CreateShaderResourceView(m_pDiffIrradianceTex.Get(), &srvd, m_pDiffIrradianceSrv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pDiffIrradianceSrv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pDiffIrradianceSrv") - 1, "D3D11::m_pDiffIrradianceSrv");


		srvd.TextureCube.MipLevels = _PREFILTER_MIP_LEVELS;
		hr = m_pDevice->CreateShaderResourceView(m_pPrefilterTex.Get(), &srvd, m_pPrefilterSrv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pPrefilterSrv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pPrefilterSrv") - 1, "D3D11::m_pPrefilterSrv");


		return true;
	}
	bool D3D11::CreateDsv(const UINT width, const UINT height,
		ID3D11Texture2D** ppDepthTex,
		ID3D11DepthStencilView** ppDepthDsv)
	{
		D3D11_TEXTURE2D_DESC depthBufferDesc;
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
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

		hr = m_pDevice->CreateTexture2D(&depthBufferDesc, nullptr, ppDepthTex);
		assert(SUCCEEDED(hr));
		(*ppDepthTex)->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::ppDepthTex") - 1, "D3D11::ppDepthTex");


		ZeroMemory(&dsvd, sizeof(dsvd));

		dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvd.Texture2D.MipSlice = 0;


		hr = m_pDevice->CreateDepthStencilView(*ppDepthTex, &dsvd, ppDepthDsv);
		assert(SUCCEEDED(hr));
		(*ppDepthDsv)->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::ppDepthDsv") - 1, "D3D11::ppDepthDsv");

		return true;
	}
	bool D3D11::CreateDss()
	{
		D3D11_DEPTH_STENCIL_DESC dsd;
		HRESULT hr;

		ZeroMemory(&dsd, sizeof(dsd));

		//Need depthStencilState to control depthStencilBuffer
		//To create it, fill out dsd
		dsd.DepthEnable = true;
		dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsd.DepthFunc = D3D11_COMPARISON_LESS;
		dsd.StencilEnable = true;
		dsd.StencilReadMask = 0xFF;
		dsd.StencilWriteMask = 0xFF;
		dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		hr = m_pDevice->CreateDepthStencilState(&dsd, m_pOutlinerSetupDss.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pOutlinerSetupDss->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pOutlinerSetupDss") - 1, "D3D11::m_pOutlinerSetupDss");


		dsd.DepthEnable = true;
		dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsd.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
		dsd.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;

		hr = m_pDevice->CreateDepthStencilState(&dsd, m_pOutlinerTestDss.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pOutlinerTestDss->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D11::m_pOutlinerTestDss") - 1, "D3D11::m_pOutlinerTestDss");

		return true;
	}
	void D3D11::DrawObject(const bool bGeoPass,const bool bSpotShadowPass)
	{	
		float color[4] = { 0.0f, 0.0f,0.0f, 1.0f };
		HRESULT hr;
		XMMATRIX worldMat, invWorldMat;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		UINT stride = sizeof(XMFLOAT3);
		UINT offset = 0;
		UINT nEntity = 0;
		UINT nEntityDrawn = 0;
		Plane* pPlanes = m_pFrustum->GetPlanes();
		
		for (int i = 0; i < m_pObjects.size(); ++i)
		{	

			std::vector<Mesh11*> pMeshes = m_pObjects[i]->GetMeshes();
			{
				{
					for (int j = 0; j < pMeshes.size(); ++j)
					{	
						nEntity++;
						worldMat = pMeshes[j]->GetTransformMatrix(false);
						invWorldMat = pMeshes[j]->GetInverseWorldMatrix();
						AABB* aabb = pMeshes[j]->GetAabb();
						if (aabb->IsOnFrustum(pPlanes, worldMat))
						{	
							nEntityDrawn++;
							m_pMatricesCb->SetWorldMatrix(&worldMat);
							m_pMatricesCb->SetInvWorldMatrix(&invWorldMat);
							m_pMatricesCb->UploadMatBuffer(m_pContext.Get(), bSpotShadowPass);
						    
					
							bool bSelected = (i == m_selectedObject && j == m_selectedMesh);
							if (bSelected)
							{
								m_pContext->OMSetDepthStencilState(m_pOutlinerSetupDss.Get(), 1);
							}
							

							for (int k = 0; k < pMeshes[j]->GetNumMaterial(); ++k)
							{
								pMeshes[j]->UploadBuffers(m_pContext.Get(), k, bGeoPass);
								m_pContext->DrawIndexed(pMeshes[j]->GetNumIndex(k), 0, 0);
							}

							if (bSelected)
							{
								m_pContext->OMSetDepthStencilState(0, 0);
							}
						}
					}

				}

				
			}

		}
		m_pFrustum->SetSubMeshesInFrustum(nEntityDrawn);
		m_pFrustum->SetSubMeshesInScene(nEntity);
	}

	void D3D11::DestroyDss()
	{
		if (m_pOutlinerSetupDss != nullptr)
		{
			m_pOutlinerSetupDss->Release();
		}

		if (m_pOutlinerTestDss != nullptr)
		{
			m_pOutlinerTestDss->Release();
		}

	}
	void D3D11::DestroyRtvSrv()
	{
		if (m_pSceneTex != nullptr)
		{
			m_pSceneTex->Release();
		}

		if (m_pSceneRtv != nullptr)
		{
			m_pSceneRtv->Release();
		}

		if (m_pSceneSrv != nullptr)
		{
			m_pSceneSrv->Release();
		}


		if (m_pSsaoTex != nullptr)
		{
			m_pSsaoTex->Release();
		}

		if (m_pSsaoRtv != nullptr)
		{
			m_pSsaoRtv->Release();
		}

		if (m_pSsaoSrv != nullptr)
		{
			m_pSsaoSrv->Release();
		}
		
		if (m_pSsaoBlurTex != nullptr)
		{
			m_pSsaoBlurTex->Release();
		}

		if (m_pSsaoBlurRtv != nullptr)
		{
			m_pSsaoBlurRtv->Release();
		}

		if (m_pSsaoBlurSrv != nullptr)
		{
			m_pSsaoBlurSrv->Release();
		}

		if (m_pSsaoBlurDebugTex != nullptr)
		{
			m_pSsaoBlurDebugTex->Release();
		}

		if (m_pSsaoBlurDebugRtv != nullptr)
		{
			m_pSsaoBlurDebugRtv->Release();
		}

		if (m_pSsaoBlurDebugSrv != nullptr)
		{
			m_pSsaoBlurDebugSrv->Release();
		}

		if (m_pBrightTex != nullptr)
		{
			m_pBrightTex->Release();
		}


		if (m_pBrightRtv != nullptr)
		{
			m_pBrightRtv->Release();
		}

		if (m_pBrightSrv != nullptr)
		{
			m_pBrightSrv->Release();
		}

		if (m_pViewportTex != nullptr)
		{
			m_pViewportTex->Release();
		}

		if (m_pViewportRtv != nullptr)
		{
			m_pViewportRtv->Release();
		}

		if (m_pViewportSrv != nullptr)
		{
			m_pViewportSrv->Release();
		}


		for (int i = 0; i < 2; ++i)
		{
			if (m_pPingPongTex[i] != nullptr)
			{
				m_pPingPongTex[i]->Release();
			}

			if (m_pPingPongRtvs[i] != nullptr)
			{
				m_pPingPongRtvs[i]->Release();
			}

			if (m_pPingPongSrvs[i] != nullptr)
			{
				m_pPingPongSrvs[i]->Release();
			}
		}
		for (int i = 0; i < static_cast<UINT>(eGbuf::cnt); ++i)
		{
			if (m_pGbufferTex[i] != nullptr)
			{
				m_pGbufferTex[i]->Release();
			}

			if (m_pGbufferRtvs[i] != nullptr)
			{
				m_pGbufferRtvs[i]->Release();
			}

			if (m_ppGbufferSrvs[i] != nullptr)
			{
				m_ppGbufferSrvs[i]->Release();
			}
		}


		if (m_pSkyBoxTex != nullptr)
		{
			m_pSkyBoxTex->Release();
		}
		if (m_pSkyBoxRtv != nullptr)
		{
			m_pSkyBoxRtv->Release();
		}
		if (m_pSkyBoxSrv != nullptr)
		{
			m_pSkyBoxSrv->Release();
		}
		
		
		if (m_pDiffIrradianceTex != nullptr)
		{
			m_pDiffIrradianceTex->Release();
		}
		if (m_pDiffIrradianceRtv != nullptr)
		{
			m_pDiffIrradianceRtv->Release();
		}
		if (m_pDiffIrradianceSrv != nullptr)
		{
			m_pDiffIrradianceSrv->Release();
		}

		if (m_pPrefilterTex != nullptr)
		{
			m_pPrefilterTex->Release();
		}
		if (m_pPrefilterRtv != nullptr)
		{
			m_pPrefilterRtv->Release();
		}
		if (m_pPrefilterSrv != nullptr)
		{
			m_pPrefilterSrv->Release();
		}


		if (m_pBrdfTex != nullptr)
		{	
			m_pBrdfTex->Release();
		}
		if (m_pBrdfRtv != nullptr)
		{
			m_pBrdfRtv->Release();
		}
		if (m_pBrdfSrv != nullptr)
		{
			m_pBrdfSrv->Release();
		}

	}
	void D3D11::DestroyHdr()
	{

		if (m_pHdrTex != nullptr)
		{
			m_pHdrTex->Release();
		}
		if (m_pHdrSrv != nullptr)
		{
			m_pHdrSrv->Release();
		}
	}
	void D3D11::DestroySceneDepthTexDsv()
	{
		if (m_pSceneDepthTex != nullptr)
		{
			m_pSceneDepthTex->Release();
		}

		if (m_SceneDsv != nullptr)
		{
			m_SceneDsv->Release();
		}
	}
	void D3D11::DestroyBackBuffer()
	{
		if (m_pScreenDepthTex != nullptr)
		{
			m_pScreenDepthTex->Release();
		}

		if (m_pScreenRtv != nullptr)
		{
			m_pScreenRtv->Release();
		}

		if (m_ScreenDsv != nullptr)
		{
			m_ScreenDsv->Release();
		}
	}
	
}