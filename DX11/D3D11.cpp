#include "D3D11.h"
#include  "../ImGui/imgui_impl_dx11.h"

namespace wilson
{
	D3D11::D3D11()
	{
		m_pSwapChain = nullptr;
		m_pDevice = nullptr;
		m_pContext = nullptr;

		m_pSkyBoxVertices = nullptr;
		m_pSkyBoxIndices = nullptr;
		m_pSkyBoxDSS = nullptr;
		m_pSkyBoxRS = nullptr;

		m_pRenderTargetView = nullptr;
		m_pDSBuffer = nullptr;
		m_pDSBufferForRTT = nullptr;
		m_pDefualtDSS = nullptr;
		m_pMirroMarkDSS = nullptr;
		m_pDrawReflectionDSS = nullptr;
		m_pRTT = nullptr;
		m_pRTTV = nullptr;
		m_pSRVForRTT = nullptr;
		m_pSkyBoxSRV = nullptr;
		m_pDSV = nullptr;
		m_pDSVforRTT = nullptr;
		m_pRS = nullptr;
		m_pRasterStateCC = nullptr;

		m_pSampleState = nullptr;

		m_pNoRenderTargetWritesBS = nullptr;
		m_pTransparentBS = nullptr;

		m_pTerrain = nullptr;
		m_pImporter = nullptr;
		m_pCam = nullptr;
		m_pFrustum = nullptr;
		m_pMatBuffer = nullptr;
		m_pLight = nullptr;
		m_pShader = nullptr;
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
		D3D11_VIEWPORT viewport;
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

		hr = m_pDevice->CreateRenderTargetView(pBackbuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(hr))
		{
			return false;
		}

		pBackbuffer->Release();
		pBackbuffer = nullptr;
		{	
			XMFLOAT3 vertices[] = {
			 {XMFLOAT3(-1.0f,  1.0f, -1.0f)},//front-upper-left  0
			 {XMFLOAT3(1.0f,   1.0f, -1.0f)},//front-upper-right 1
			 {XMFLOAT3(1.0f,  -1.0f, -1.0f)},//front-down-right  2
			 {XMFLOAT3(-1.0f, -1.0f, -1.0f)},//front-down-left   3

			 {XMFLOAT3(-1.0f,  1.0f, 1.0f)},//back-upper-left   4
			 {XMFLOAT3(1.0f,   1.0f, 1.0f)},//back-upper-right  5
			 {XMFLOAT3(1.0f,  -1.0f, 1.0f)},//back-down-right   6
			 {XMFLOAT3(-1.0f, -1.0f, 1.0f)},//back-down-left    7

			 {XMFLOAT3(-1.0f,  1.0f, -1.0f)},//front-upper-left  8
			 {XMFLOAT3(-1.0f, -1.0f, -1.0f)},//front-down-left   9
			 {XMFLOAT3(-1.0f,  1.0f, 1.0f)},//back-upper-left   10
			 {XMFLOAT3(-1.0f, -1.0f, 1.0f)},//back-down-left    11

			 {XMFLOAT3(1.0f,   1.0f, -1.0f)},//front-upper-right 12
			 {XMFLOAT3(1.0f,  -1.0f, -1.0f)},//front-down-right  13
			 {XMFLOAT3(1.0f,   1.0f, 1.0f)},//back-upper-right  14
			 {XMFLOAT3(1.0f,  -1.0f, 1.0f)},//back-down-right   15

			 {XMFLOAT3(-1.0f,  1.0f, -1.0f)},//front-upper-left  16
			 {XMFLOAT3(1.0f,   1.0f, -1.0f)},//front-upper-right 17
			 {XMFLOAT3(-1.0f,  1.0f, 1.0f)},//back-upper-left   18
			 {XMFLOAT3(1.0f,   1.0f, 1.0f)},//back-upper-right  19

			 {XMFLOAT3(1.0f,  -1.0f, -1.0f)},//front-down-right  20
			 {XMFLOAT3(-1.0f, -1.0f, -1.0f)},//front-down-left   21
			 {XMFLOAT3(1.0f,  -1.0f, 1.0f)},//back-down-right   22
			 {XMFLOAT3(-1.0f, -1.0f, 1.0f)} };//back-down-left    23
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

		    hr = m_pDevice->CreateBuffer(&skyBoxVertexBD,&skyBoxVertexData, &m_pSkyBoxVertices);
			if (FAILED(hr))
			{
				return false;
			}

			unsigned long indices[] = {
	   0,1,2,
	   0,2,3,

	   4,5,6,
	   4,6,7,

	   8,10,11,
	   8,11,9,

	   12,14,15,
	   12,15,13,

	   16,18,19,
	   16,19,17,

	   21,23,22,
	   21,22,20

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

			hr = m_pDevice->CreateBuffer(&skyBoxIndexBD, &skyBoxIndexData, &m_pSkyBoxIndices);
			if(FAILED(hr))
			{
				return false;
			}

			hr = D3DX11CreateShaderResourceViewFromFileW(m_pDevice, L"./Assets/Textures/ocean.dds",
				0, 0, &m_pSkyBoxSRV, 0);
			if (FAILED(hr))
			{
				return false;
			}

			D3D11_DEPTH_STENCIL_DESC skyboxDSD;
			ZeroMemory(&skyboxDSD, sizeof(D3D11_DEPTH_STENCIL_DESC));
			skyboxDSD.DepthEnable = true;
			skyboxDSD.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			skyboxDSD.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			hr = m_pDevice->CreateDepthStencilState(&skyboxDSD, &m_pSkyBoxDSS);
			if (FAILED(hr))
			{
				return false;
			}

			D3D11_RASTERIZER_DESC skyboxRD;
			ZeroMemory(&skyboxRD, sizeof(D3D11_RASTERIZER_DESC));
			skyboxRD.CullMode = D3D11_CULL_NONE;
			skyboxRD.FillMode = D3D11_FILL_SOLID;
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

		if (!CreateDepthBuffer(m_clientWidth, m_clientHeight, &m_pDSBuffer, &m_pDSV))
		{
			return false;
		}
		
		if (!CreateDepthBuffer(m_clientWidth, m_clientHeight, &m_pDSBufferForRTT, &m_pDSVforRTT))
		{
			return false;
		}

		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;

		hr = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pRS);
		if (FAILED(hr))
		{
			return false;
		}

		m_pContext->RSSetState(m_pRS);
		
		viewport.Width = static_cast<float>(screenWidth);
		viewport.Height = static_cast<float>(screenHeight);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;

		m_pContext->RSSetViewports(1, &viewport);

		//Set projectionMatrix, viewMatrix;
		m_pTerrain = new Terrain;
		m_pTerrain->Init(m_pDevice, 100, 100);
		m_pCam = new Camera(screenWidth, screenHeight, fScreenFar, fScreenNear);
		m_pCam->Init(m_pDevice);
		m_pCam->SetCamBuffer(m_pContext);
		XMMATRIX* m_projMat = m_pCam->GetProjectionMatrix();
		XMMATRIX* m_viewMat = m_pCam->GetViewMatrix();
		m_pFrustum = new Frustum();
		m_pFrustum->Construct(100.0f, m_pCam);
		m_pMatBuffer = new MatBuffer(m_pDevice, m_pContext, m_projMat, m_viewMat);
		m_pMatBuffer->Init();

		m_pLight = new Light(m_pDevice, m_pContext);
		m_pLight->Init();
		m_pLight->Update();

		//m_pCImporter = new CImporter();
		//m_pCImporter->LoadOBJ(L"./Models/sphere/Sphere.obj");
		//m_ppCModels.push_back(m_pCImporter->GetModel());
		//m_pCImporter->LoadTex(m_ppCModels[0], L"./Models/sphere/Sphere.png", m_pDevice);
		//m_ppCModels[0]->Init(m_pDevice);
		//m_pCImporter->Clear();

		m_pShader = new Shader(m_pDevice, m_pContext);
		m_pShader->Init();


		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		hr = m_pDevice->CreateSamplerState(&samplerDesc, &m_pSampleState);
		if (FAILED(hr))
		{
			return false;
		}
		m_pContext->PSSetSamplers(0, 1, &m_pSampleState);

		rtBlendDSC.BlendEnable = TRUE;
		rtBlendDSC.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		rtBlendDSC.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		rtBlendDSC.BlendOp = D3D11_BLEND_OP_ADD;
		rtBlendDSC.SrcBlendAlpha = D3D11_BLEND_ONE;
		rtBlendDSC.DestBlendAlpha = D3D11_BLEND_ONE;
		rtBlendDSC.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtBlendDSC.RenderTargetWriteMask = 0;

		D3D11_BLEND_DESC blendDSC = { false, false, rtBlendDSC };
		hr = m_pDevice->CreateBlendState(&blendDSC, &m_pNoRenderTargetWritesBS);
		if (FAILED(hr))
		{
			return false;
		}

		rtBlendDSC.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blendDSC.RenderTarget[0] = rtBlendDSC;
		hr = m_pDevice->CreateBlendState(&blendDSC, &m_pTransparentBS);
		if (FAILED(hr))
		{
			return false;
		}

		float blendV[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_pContext->OMSetBlendState(m_pTransparentBS, blendV, 0xffffffff);
		m_pContext->OMSetDepthStencilState(m_pDefualtDSS, 1);
		ImGui_ImplDX11_Init(m_pDevice, m_pContext);
		return true;
	}

	void D3D11::Shutdown()
	{
		ImGui_ImplDX11_Shutdown();

		if (m_pSkyBoxVertices != nullptr)
		{
			m_pSkyBoxVertices->Release();
			m_pSkyBoxVertices = nullptr;
		}

		if (m_pSkyBoxIndices != nullptr)
		{
			m_pSkyBoxIndices->Release();
			m_pSkyBoxIndices = nullptr;
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

		if (m_pRS != nullptr)
		{
			m_pRS->Release();
			m_pRS = nullptr;
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

		if (m_pDSV != nullptr)
		{
			m_pDSV->Release();
			m_pDSV = nullptr;
		}

		DestroyDSS();
		if (m_pSkyBoxSRV != nullptr)
		{
			m_pSkyBoxSRV->Release();
			m_pSkyBoxSRV = nullptr;
		}

		if (m_pRenderTargetView != nullptr)
		{
			m_pRenderTargetView->Release();
			m_pRenderTargetView = nullptr;
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

		if (m_pImporter != nullptr)
		{
			delete m_pImporter;
			m_pImporter = nullptr;
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

		if (m_pLight != nullptr)
		{
			delete m_pLight;
			m_pLight = nullptr;
		}

		if (m_pShader != nullptr)
		{
			delete m_pShader;
			m_pShader = nullptr;
		}

		if (m_pNoRenderTargetWritesBS != nullptr)
		{
			m_pNoRenderTargetWritesBS->Release();
			m_pNoRenderTargetWritesBS = nullptr;
		}

		if (m_pTransparentBS != nullptr)
		{
			m_pTransparentBS->Release();
			m_pTransparentBS = nullptr;
		}

		for (int i = 0; i < m_ppModels.size(); ++i)
		{
			if (m_ppModels[i] != nullptr)
			{
				delete m_ppModels[i];
			}
		}
		m_ppModels.clear();
		m_ppModels.shrink_to_fit();
		return;
	}

	void D3D11::UpdateScene()
	{
		//clear views
		HRESULT hr;
		XMMATRIX m_worldMat = XMMatrixTranslationFromVector(XMVectorSet(-50.0f, 5.0f, -1.0f, 1.0f));
		float color[4] = { 0.0f, 0.0f,0.0f, 1.0f };
		UINT stride = 0;
		UINT offset = 0;
		int drawed = 0;

		m_pContext->ClearRenderTargetView(m_pRenderTargetView, color);
		m_pContext->ClearRenderTargetView(m_pRTTV, color);
		m_pContext->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
		m_pContext->ClearDepthStencilView(m_pDSVforRTT, D3D11_CLEAR_DEPTH, 1.0f, 0);
		m_pContext->OMSetRenderTargets(1, &m_pRTTV, m_pDSVforRTT);
		//Update Light
		m_pLight->Update();
		//Update Cam 
		m_pCam->Update();
		m_pFrustum->Construct(100.0f, m_pCam);
		/*Draw Terrain
		m_pCTerrain->UploadBuffers(m_pContext);
		m_pCMBuffer->SetViewMatrix(m_pCCam->GetViewMatrix());
		m_pCMBuffer->SetWorldMatrix(&world);
		m_pCMBuffer->Update();
		m_pContext->DrawIndexed(m_pCTerrain->GetIndexCount(), 0, 0);
		*/
		
		/*/Draw EnvMap
		m_pMatBuffer->SetWorldMatrix(&m_idMat);	
		m_pMatBuffer->Update();
		m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_pShader->SetSkyBoxInputLayout();
		m_pShader->SetSkyBoxShader();
		m_pContext->IASetVertexBuffers(0, 1, &m_pSkyBoxVertices, &stride, &offset);
		m_pContext->IASetIndexBuffer(m_pSkyBoxIndices, DXGI_FORMAT_R32_UINT, 0);
		m_pContext->PSSetShaderResources(0, 1, &m_pSkyBoxSRV);
		m_pContext->RSSetState(m_pSkyBoxRS);
		//m_pContext->OMSetDepthStencilState(m_pSkyBoxDSS, 0);
		m_pContext->DrawIndexed(30, 0, 0);*/
		//Draw ENTTs
		m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pShader->SetInputLayout();
		m_pShader->SetShader();
		m_pContext->RSSetState(m_pRS);
		m_pContext->OMSetDepthStencilState(m_pDefualtDSS, 1);
		for (int i = 0; i < m_ppModels.size(); ++i)
		{
			m_worldMat = m_ppModels[i]->GetTransformMatrix();
			m_pMatBuffer->SetViewMatrix(m_pCam->GetViewMatrix());
			XMFLOAT4X4 pos4;
			XMStoreFloat4x4(&pos4, m_worldMat);
			if (m_pFrustum->IsInFrustum(XMVectorSet(pos4._41, pos4._42, pos4._43, pos4._44)))
			{
				m_pMatBuffer->SetWorldMatrix(&m_worldMat);
				m_pMatBuffer->Update();

				if (m_ppModels[i]->GetObjectType() == EObjectType::FBX)
				{
					m_pShader->SetInputLayout();
					std::vector<unsigned int> indicesCount = m_ppModels[i]->GetNumIndice();
					if (m_ppModels[i]->isInstanced())
					{
						std::vector<unsigned int> verticesCount = m_ppModels[i]->GetNumVertexData();
						m_pShader->SetInputLayout();
						int numInstance = m_ppModels[i]->GetNumInstance();
						for (int j = 0; j < verticesCount.size(); ++j)
						{
							m_ppModels[i]->UploadBuffers(m_pContext, j);
							m_pContext->DrawInstanced(verticesCount[j], numInstance, 0, 0);
						}
					}
					
					else
					{	
						for (int j = 0; j < indicesCount.size(); ++j)
						{
							m_ppModels[i]->UploadBuffers(m_pContext, j);
							m_pContext->Draw(indicesCount[j], 0);
						}
					}
			
				}
				else
				{	
					m_ppModels[i]->UploadBuffers(m_pContext);
					m_pContext->DrawIndexed(m_ppModels[i]->GetIndexCount(), 0, 0);
				}
				
				++drawed;
			}

		}
		m_pCam->SetENTTsInFrustum(drawed);
		m_pContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDSV);
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

	void D3D11::AddModel(Model* pCModel, ID3D11Device* pDevice)
	{
		m_ppModels.push_back(pCModel);
		m_ppModels.back()->Init(pDevice);
	}

	void D3D11::RemoveModel(int i)
	{
		delete m_ppModels[i];
		m_ppModels.erase(m_ppModels.begin() + i);
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

		hr = m_pDevice->CreateTexture2D(&RTTDesc, nullptr, &m_pRTT);
		if (FAILED(hr))
		{
			return false;
		}

		RTTVDesc.Format = RTTDesc.Format;
		RTTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		RTTVDesc.Texture2D.MipSlice = 0;

		hr = m_pDevice->CreateRenderTargetView(m_pRTT, &RTTVDesc, &m_pRTTV);
		if (FAILED(hr))
		{
			return false;
		}


		SRVDesc.Format = RTTDesc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = m_pDevice->CreateShaderResourceView(m_pRTT, &SRVDesc, &m_pSRVForRTT);
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
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		hr = m_pDevice->CreateDepthStencilState(&depthStencilDesc, &m_pDefualtDSS);
		if (FAILED(hr))
		{
			return false;
		}


		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		hr = m_pDevice->CreateDepthStencilState(&depthStencilDesc, &m_pMirroMarkDSS);
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

	void D3D11::DestroyDSS()
	{
		if (m_pDefualtDSS != nullptr)
		{
			m_pDefualtDSS->Release();
			m_pDefualtDSS = nullptr;
		}

		if (m_pMirroMarkDSS != nullptr)
		{
			m_pMirroMarkDSS->Release();
			m_pMirroMarkDSS = nullptr;
		}

		if (m_pDrawReflectionDSS != nullptr)
		{
			m_pDrawReflectionDSS->Release();
			m_pDrawReflectionDSS = nullptr;
		}
	}

	void D3D11::DestroyRTT()
	{
		if (m_pRTT != nullptr)
		{
			m_pRTT->Release();
			m_pRTT = nullptr;
		}

		if (m_pRTTV != nullptr)
		{
			m_pRTTV->Release();
			m_pRTTV = nullptr;
		}

		if (m_pSRVForRTT != nullptr)
		{
			m_pSRVForRTT->Release();
			m_pSRVForRTT = nullptr;
		}
	}

	void D3D11::DestroyDSBforRTT()
	{
		if (m_pDSBufferForRTT != nullptr)
		{
			m_pDSBufferForRTT->Release();
			m_pDSBufferForRTT = nullptr;
		}

		if (m_pDSVforRTT != nullptr)
		{
			m_pDSVforRTT->Release();
			m_pDSVforRTT = nullptr;
		}
	}

}