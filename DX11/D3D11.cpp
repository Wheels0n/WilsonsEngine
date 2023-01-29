#include "D3D11.h"
#include  "../ImGui/imgui_impl_dx11.h"
CD3D11::CD3D11()
{
	m_pSwapChain = nullptr;
	m_pDevice = nullptr;
	m_pContext = nullptr;
	m_pRenderTargetView = nullptr;
	m_pDSBuffer = nullptr;
	m_pDSBufferForRTT = nullptr;
	m_pDefualtDDS = nullptr;
	m_pMirroMarkDDS = nullptr;
	m_pDrawReflectionDDS = nullptr;
	m_pRTT = nullptr;
	m_pRTTV = nullptr;
	m_pSRVForRTT = nullptr;
	m_pDepthStencilView = nullptr;
	m_pDSVforRTT = nullptr;
	m_pRasterstate = nullptr;
	m_pRasterStateCC = nullptr;

	m_pSampleState = nullptr;

	m_pNoRenderTargetWritesBS = nullptr;
	m_pTransparentBS = nullptr;

	m_pCTerrain = nullptr;
	m_pCImporter = nullptr;
	m_pCCam  = nullptr;
	m_pCFrustum = nullptr;
	m_pCMBuffer = nullptr;
	m_pCWMTransformation = nullptr;
	m_pCLight = nullptr;
	m_pCShader = nullptr;
}

CD3D11::~CD3D11()
{
}

bool CD3D11::Init(int screenWidth, int screenHeight, bool bVsync, HWND hWnd, bool bFullscreen,
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
	D3D11_RENDER_TARGET_BLEND_DESC rtBlendDsc;
	float fFOV, fScreenAspect;
	m_bVsync_enabled = bVsync;

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


	m_clientWidth = screenWidth;
	m_clientHeight = screenHeight;

	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;


	if (m_bVsync_enabled == true)
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
    
	if (!CreateRTT(m_clientWidth, m_clientHeight))
	{
		return false;
	}

	if (!CreateDSS())
	{
		return false;
	}

	if (!CreateDepthBuffer(m_clientWidth, m_clientHeight, &m_pDSBuffer, &m_pDepthStencilView))
	{
		return false;
	}

	if (!CreateDepthBuffer(m_clientWidth, m_clientHeight, &m_pDSBufferForRTT, &m_pDSVforRTT))
	{
		return false;
	}

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode =D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	hr = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pRasterstate);
	if (FAILED(hr))
	{
		return false;
	}

	m_pContext->RSSetState(m_pRasterstate);

	viewport.Width = static_cast<float>(screenWidth);
	viewport.Height = static_cast<float>(screenHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	m_pContext->RSSetViewports(1, &viewport);

	//Set projectionMatrix, viewMatrix;
	m_pCTerrain = new CTerrain;
	m_pCTerrain->Init(m_pDevice, 100, 100);
	m_pCCam = new CCamera(screenWidth, screenHeight, fScreenFar, fScreenNear);
	m_pCCam->Init(m_pDevice);
	m_pCCam->SetCamBuffer(m_pContext);
	XMMATRIX* projection=  m_pCCam->GetProjectionMatrix();
	XMMATRIX* view = m_pCCam->GetViewMatrix();
	m_pCFrustum = new CFrustum();
	m_pCFrustum->Construct(100.0f, m_pCCam);
	m_pCMBuffer = new CMBuffer(m_pDevice, m_pContext, projection, view);
	m_pCMBuffer->Init();
	m_pCWMTransformation = new CWMTransformation(m_pDevice, m_pContext);
	
	m_pCLight = new CLight(m_pDevice, m_pContext);
	m_pCLight->Init();
	m_pCLight->Update();

	//m_pCImporter = new CImporter();
	//m_pCImporter->LoadOBJ(L"./Models/sphere/Sphere.obj");
	//m_ppCModels.push_back(m_pCImporter->GetModel());
	//m_pCImporter->LoadTex(m_ppCModels[0], L"./Models/sphere/Sphere.png", m_pDevice);
	//m_ppCModels[0]->Init(m_pDevice);
	//m_pCImporter->Clear();
	
	m_pCShader = new CShader(m_pDevice, m_pContext);
	m_pCShader-> Init();


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
	m_pContext->PSSetSamplers(0, 1, &m_pSampleState);

		rtBlendDsc.BlendEnable = true;
		rtBlendDsc.SrcBlend = D3D11_BLEND_BLEND_FACTOR;
		rtBlendDsc.DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
		rtBlendDsc.BlendOp = D3D11_BLEND_OP_ADD;
		rtBlendDsc.SrcBlendAlpha = D3D11_BLEND_ONE;
		rtBlendDsc.DestBlendAlpha = D3D11_BLEND_ZERO;
		rtBlendDsc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtBlendDsc.RenderTargetWriteMask = 0;

		D3D11_BLEND_DESC blendDsc= { false, false, rtBlendDsc };
		hr = m_pDevice->CreateBlendState(&blendDsc, &m_pNoRenderTargetWritesBS);
		if (FAILED(hr))
		{
			return false;
		}

		rtBlendDsc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blendDsc.RenderTarget[0] = rtBlendDsc;
		hr = m_pDevice->CreateBlendState(&blendDsc, &m_pTransparentBS);
		if (FAILED(hr))
		{
			return false;
		}

		float blendV[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
		m_pContext->OMSetBlendState(nullptr, blendV, 0xf);
		m_pContext->OMSetDepthStencilState(m_pDefualtDDS, 1);
		ImGui_ImplDX11_Init(m_pDevice, m_pContext);
	return true;
}

void CD3D11::Shutdown()
{   
	ImGui_ImplDX11_Shutdown();

	if (m_pSwapChain!=nullptr)
	{
		m_pSwapChain->SetFullscreenState(false, nullptr);
	}

	if (m_pRasterstate != nullptr)
	{
		m_pRasterstate->Release();
		m_pRasterstate = nullptr;
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

	if (m_pDepthStencilView != nullptr)
	{
		m_pDepthStencilView->Release();
		m_pDepthStencilView = nullptr;
	}

	DestroyDSS();

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

	if (m_pCImporter != nullptr)
	{
		delete m_pCImporter;
		m_pCImporter = nullptr;
	}

	if (m_pCWMTransformation != nullptr)
	{
		delete m_pCWMTransformation;
		m_pCWMTransformation = nullptr;
	}

	if (m_pCMBuffer != nullptr)
	{
		delete m_pCMBuffer;
		m_pCMBuffer = nullptr;
	}

	if (m_pCTerrain != nullptr)
	{
		delete m_pCTerrain;
		m_pCTerrain = nullptr;
	}

	if (m_pCCam != nullptr)
	{
		delete m_pCCam;
		m_pCCam = nullptr;
	}
    
	if (m_pCFrustum != nullptr)
	{
		delete m_pCFrustum;
		m_pCFrustum = nullptr;
	}

	if (m_pCLight != nullptr)
	{   
		delete m_pCLight;
		m_pCLight = nullptr;
	}

	if (m_pCShader == nullptr)
	{  
		delete m_pCShader;
		m_pCShader = nullptr;
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

	for (int i = 0; i < m_ppCModels.size(); ++i)
	{
		if (m_ppCModels[i] != nullptr)
		{
			delete m_ppCModels[i];
		}
	}
	m_ppCModels.clear();
	m_ppCModels.shrink_to_fit();
	return;
}


void CD3D11::UpdateScene()
{
	//clear views
	HRESULT hr;
	XMMATRIX world = XMMatrixTranslationFromVector(XMVectorSet(-50.0f, 5.0f, -1.0f, 1.0f));
	float color[4] = { 0.0f, 0.0f,0.0f, 1.0f };
	int drawed = 0;

	m_pContext->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pContext->ClearRenderTargetView(m_pRTTV, color);
	m_pContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_pContext->ClearDepthStencilView(m_pDSVforRTT, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_pContext->OMSetRenderTargets(1, &m_pRTTV, m_pDSVforRTT);

	//Update Light
	m_pCLight->Update();
	//Update Cam 
	m_pCCam->Update();
	m_pCFrustum->Construct(100.0f, m_pCCam);
	/*Draw Terrain
	m_pCTerrain->UploadBuffers(m_pContext);
	m_pCMBuffer->SetViewMatrix(m_pCCam->GetViewMatrix());
	m_pCMBuffer->SetWorldMatrix(&world);
	m_pCMBuffer->Update();
	m_pContext->DrawIndexed(m_pCTerrain->GetIndexCount(), 0, 0);
	*/
	//Draw ENTTs
	for (int i = 0; i < m_ppCModels.size(); ++i)
	{   
		world = m_ppCModels[i]->GetTransformMatrix();
		XMFLOAT4X4 pos4;
		XMStoreFloat4x4(&pos4, world);
		if (m_pCFrustum->IsInFrustum(XMVectorSet(pos4._41, pos4._42, pos4._43, pos4._44)))
		{  
			m_pCMBuffer->SetViewMatrix(m_pCCam->GetViewMatrix());
			m_pCMBuffer->SetWorldMatrix(&world);
			m_pCMBuffer->Update();
			m_ppCModels[i]->UploadBuffers(m_pContext);
			m_pContext->DrawIndexed(m_ppCModels[i]->GetIndexCount(), 0, 0);

			++drawed;
		}
		
	}
	m_pCCam->SetENTTsInFrustum(drawed);
	m_pContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
	return;
}

void CD3D11::DrawScene()
{   
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}


	if (m_bVsync_enabled == true)
	{
		m_pSwapChain->Present(1, 0);
	}
	else
	{
		m_pSwapChain->Present(0, 0);
	}
}

void CD3D11::AddModel(CModel* pCModel, ID3D11Device* pDevice)
{
	m_ppCModels.push_back(pCModel);
	m_ppCModels.back()->Init(pDevice);
}

void CD3D11::RemoveModel(int i)
{  
	delete m_ppCModels[i];
	m_ppCModels.erase(m_ppCModels.begin() + i);
}

bool CD3D11::CreateRTT(int width, int height)
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

bool CD3D11::CreateDepthBuffer(int width, int height, 
	ID3D11Texture2D** dsb, 
	ID3D11DepthStencilView** dsbv )
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

bool CD3D11::CreateDSS()
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

	hr = m_pDevice->CreateDepthStencilState(&depthStencilDesc, &m_pDefualtDDS);
	if (FAILED(hr))
	{
		return false;
	}


	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	hr = m_pDevice->CreateDepthStencilState(&depthStencilDesc, &m_pMirroMarkDDS);
	if (FAILED(hr))
	{
		return false;
	}

	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	hr = m_pDevice->CreateDepthStencilState(&depthStencilDesc, &m_pDrawReflectionDDS);
	if (FAILED(hr))
	{
		return false;
	}
	
	return true;
}

void CD3D11::DestroyDSS()
{
	if (m_pDefualtDDS != nullptr)
	{
		m_pDefualtDDS->Release();
		m_pDefualtDDS = nullptr;
	}

	if (m_pMirroMarkDDS != nullptr)
	{
		m_pMirroMarkDDS->Release();
		m_pMirroMarkDDS = nullptr;
	}

	if (m_pDrawReflectionDDS != nullptr)
	{
		m_pDrawReflectionDDS->Release();
		m_pDrawReflectionDDS = nullptr;
	}
}

void CD3D11::DestroyRTT()
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

void CD3D11::DestroyDSBforRTT()
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
