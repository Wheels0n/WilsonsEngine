#include "D3D11.h"

CD3D11::CD3D11()
{
	m_pSwapChain = nullptr;
	m_pDevice = nullptr;
	m_pContext = nullptr;
	m_pRenderTargetView = nullptr;
	m_pDepthStencilBuffer = nullptr;
	m_pDepthStencilState = nullptr;
	m_pDepthStencilView = nullptr;
	m_pRasterstate = nullptr;

	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;
	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
	m_pInputLayout = nullptr;
	m_pShaderResourceView = nullptr;
	m_pSampleState = nullptr;
	m_LightBuffer = nullptr;
}

CD3D11::~CD3D11()
{
}

bool CD3D11::Init(int screenWidth, int screenHeight, bool bVsync, HWND hWnd, bool bFullscreen,
	float fScreenDepth, float fScreenNear)
{

	HRESULT hr;
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
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
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
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
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

	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	hr = m_pDevice->CreateTexture2D(&depthBufferDesc, nullptr, &m_pDepthStencilBuffer);
	if (FAILED(hr))
	{
		return false;
	}

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

	hr = m_pDevice->CreateDepthStencilState(&depthStencilDesc, &m_pDepthStencilState);
	if (FAILED(hr))
	{
		return false;
	}

	m_pContext->OMSetDepthStencilState(m_pDepthStencilState, 1);

	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;


	hr = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(hr))
	{
		return false;
	}

	m_pContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

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
	fFOV = static_cast<float>(D3DX_PI) / 4.0f;
	fScreenAspect = screenWidth / static_cast<float>(screenHeight);
	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fFOV, fScreenAspect, fScreenNear, fScreenDepth);
	
	D3DXVECTOR3 m_vPos = { 0.0f,-3.0f,-6.0f };  //Translation
	D3DXVECTOR3 m_vLookat = { 0.0f, 1.0f, 0.0f };//camera look-at target
	D3DXVECTOR3 m_vUp = { 0.0f, 1.0f, 0.0f };   //which axis is upward
	D3DXMatrixLookAtLH(&m_viewMatrix, &m_vPos, &m_vLookat, &m_vUp);


	//Set vertexData, indexData
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_INPUT_ELEMENT_DESC vertexDesc[3];
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	ID3DBlob* pVsBlob, * pPsBlob, * pErrorBlob;


	VertexType vertices[] = { 
	{D3DXVECTOR3(-1.0f,  1.0f, -1.0f), D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 1.0f)},//front-upper-left  0
	{D3DXVECTOR3(1.0f,   1.0f, -1.0f), D3DXVECTOR2(0.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, 1.0f)},//front-upper-right 1  WORK
	{D3DXVECTOR3(1.0f,  -1.0f, -1.0f), D3DXVECTOR2(1.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, 1.0f)},//front-down-right  2
	{D3DXVECTOR3(-1.0f, -1.0f, -1.0f), D3DXVECTOR2(1.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 1.0f)},//front-down-left   3

	{D3DXVECTOR3(-1.0f,  1.0f, 1.0f), D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, -1.0f)},//back-upper-left   4
	{D3DXVECTOR3(1.0f,   1.0f, 1.0f), D3DXVECTOR2(0.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, -1.0f)},//back-upper-right  5
	{D3DXVECTOR3(1.0f,  -1.0f, 1.0f), D3DXVECTOR2(1.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, -1.0f)},//back-down-right   6
	{D3DXVECTOR3(-1.0f, -1.0f, 1.0f), D3DXVECTOR2(1.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, -1.0f)},//back-down-left    7  WORK

	{D3DXVECTOR3(-1.0f,  1.0f, -1.0f), D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR3(-1.0f, 0.0f, 0.0f)},//front-upper-left  8
	{D3DXVECTOR3(-1.0f, -1.0f, -1.0f), D3DXVECTOR2(0.0f, 1.0f), D3DXVECTOR3(-1.0f, 0.0f, 0.0f)},//front-down-left   9  WORK
	{D3DXVECTOR3(-1.0f,  1.0f, 1.0f), D3DXVECTOR2(1.0f, 1.0f), D3DXVECTOR3(-1.0f, 0.0f, 0.0f)},//back-upper-left   10
	{D3DXVECTOR3(-1.0f, -1.0f, 1.0f), D3DXVECTOR2(1.0f, 0.0f), D3DXVECTOR3(-1.0f, 0.0f, 0.0f)},//back-down-left    11

	{D3DXVECTOR3(1.0f,   1.0f, -1.0f), D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR3(1.0f, 0.0f, 0.0f)},//front-upper-right 12
	{D3DXVECTOR3(1.0f,  -1.0f, -1.0f), D3DXVECTOR2(0.0f, 1.0f), D3DXVECTOR3(1.0f, 0.0f, 0.0f)},//front-down-right  13 WORK
	{D3DXVECTOR3(1.0f,   1.0f, 1.0f), D3DXVECTOR2(1.0f, 1.0f), D3DXVECTOR3(1.0f, 0.0f, 0.0f)},//back-upper-right  14
	{D3DXVECTOR3(1.0f,  -1.0f, 1.0f), D3DXVECTOR2(1.0f, 0.0f), D3DXVECTOR3(1.0f, 0.0f, 0.0f)},//back-down-right   15

	{D3DXVECTOR3(-1.0f,  1.0f, -1.0f), D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR3(0.0f, 1.0f, 0.0f)},//front-upper-left  16
	{D3DXVECTOR3(1.0f,   1.0f, -1.0f), D3DXVECTOR2(0.0f, 1.0f), D3DXVECTOR3(0.0f, 1.0f, 0.0f)},//front-upper-right 17
	{D3DXVECTOR3(-1.0f,  1.0f, 1.0f), D3DXVECTOR2(1.0f, 1.0f), D3DXVECTOR3(0.0f, 1.0f, 0.0f)},//back-upper-left   18
	{D3DXVECTOR3(1.0f,   1.0f, 1.0f), D3DXVECTOR2(1.0f, 0.0f), D3DXVECTOR3(0.0f, 1.0f, 0.0f)},//back-upper-right  19

	{D3DXVECTOR3(-1.0f,-1.0f, -1.0f), D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f)},//front-down-left   20
	{D3DXVECTOR3(1.0f, -1.0f, -1.0f), D3DXVECTOR2(0.0f, 1.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f)},//front-down-right  21
	{D3DXVECTOR3(1.0f,  -1.0f, 1.0f), D3DXVECTOR2(1.0f, 1.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f)},//back-down-right   22
	{D3DXVECTOR3(-1.0f, -1.0f, 1.0f), D3DXVECTOR2(1.0f, 0.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f)},//back-down-left    23
	};

	unsigned long indices[] = { 
		0,1,2,
		0,2,3,

		5,4,7,
		5,7,6,

		10,8,9,
		10,9,11,

		12,14,15,
		12,15,13,

		16,18,19,
		16,19,17,

		20,21,22,
		20,22,23
	                          
	};
	
	m_vertexCount = sizeof(vertices) / sizeof(VertexType);
	m_indexCount = sizeof(indices) / sizeof(unsigned long);


	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].SemanticIndex = 0;
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].InputSlot = 0;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	vertexDesc[0].InstanceDataStepRate = 0;
	
	vertexDesc[1].SemanticName = "TEXTURE";
	vertexDesc[1].SemanticIndex = 0;
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].InputSlot = 0;
	vertexDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	vertexDesc[1].InstanceDataStepRate = 0;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].SemanticIndex = 0;
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].InputSlot = 0;
	vertexDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	vertexDesc[2].InstanceDataStepRate = 0;

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;


	hr = m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	
	//describe our indice
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;


	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	hr = m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	//Load VS to VS variable
	hr = D3DX10CompileFromFile(L"VS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pVsBlob, &pErrorBlob, nullptr);
	if (FAILED(hr))
	{
		return false;
	}
	m_pDevice->CreateVertexShader(pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), nullptr, &m_pVertexShader);

	hr = D3DX10CompileFromFile(L"PS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pPsBlob, &pErrorBlob, nullptr);
	if (FAILED(hr))
	{
		return false;
	}
	m_pDevice->CreatePixelShader(pPsBlob->GetBufferPointer(), pPsBlob->GetBufferSize(), nullptr, &m_pPixelShader);

	m_pDevice->CreateInputLayout(vertexDesc, sizeof(vertexDesc) / sizeof(vertexDesc[0]), pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &m_pInputLayout);
	
	
	Objects[0] = new CObject(m_pDevice, m_pContext, &m_projectionMatrix, &m_viewMatrix);
	Objects[0]->Init();
	m_pConstantBuffers[0] = Objects[0]->getCB();
	
	D3D11_BUFFER_DESC lightCbd;
	lightCbd.Usage = D3D11_USAGE_DYNAMIC;
	lightCbd.ByteWidth = sizeof(Light);
	lightCbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightCbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightCbd.MiscFlags = 0;
	lightCbd.StructureByteStride = 0;
	m_pDevice->CreateBuffer(&lightCbd, nullptr, &m_LightBuffer);


	hr = D3DX11CreateShaderResourceViewFromFileW(
		m_pDevice, L"seafloor.dds", nullptr, nullptr, &m_pShaderResourceView, nullptr);
	if (FAILED(hr))
	{
		return false;
	}
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

	pPsBlob->Release();
	pPsBlob = nullptr;

	pVsBlob->Release();
	pVsBlob = nullptr;

	if (pErrorBlob != nullptr)
	{
		pErrorBlob->Release();
		pErrorBlob = nullptr;
	}
	return true;
}

void CD3D11::Shutdown()
{
	if (m_pSwapChain!=nullptr)
	{
		m_pSwapChain->SetFullscreenState(false, nullptr);
	}

	if (m_pRasterstate != nullptr)
	{
		m_pRasterstate->Release();
		m_pRasterstate = nullptr;
	}

	if (m_pDepthStencilView != nullptr)
	{
		m_pDepthStencilView->Release();
		m_pDepthStencilView = nullptr;
	}

	if (m_pDepthStencilState != nullptr)
	{
		m_pDepthStencilState->Release();
		m_pDepthStencilState = nullptr;
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

	if (m_pIndexBuffer != nullptr)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = nullptr;
	}

	if (m_pVertexBuffer != nullptr)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}

	if (m_pVertexShader != nullptr)
	{
		m_pVertexShader->Release();
		m_pVertexShader = nullptr;
	}

	if (m_pPixelShader != nullptr)
	{
		m_pPixelShader->Release();
		m_pPixelShader = nullptr;
	}

	if (m_pInputLayout != nullptr)
	{
		m_pInputLayout->Release();
		m_pInputLayout = nullptr;
	}

	if (m_pShaderResourceView != nullptr)
	{
		m_pShaderResourceView->Release();
		m_pShaderResourceView = nullptr;
	}

	if (m_pSampleState != nullptr)
	{
		m_pSampleState->Release();
		m_pSampleState = nullptr;
	}

	return;
}


void CD3D11::UpdateScene()
{  
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currtime));

	//clear views
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	Light* pLight;
	unsigned int stride;
	unsigned int offset;
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	m_pContext->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	stride = sizeof(VertexType);
	offset = 0;

	m_pContext->Map(m_LightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	pLight = reinterpret_cast<Light*>(mappedResource.pData);
	pLight->diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
	pLight->direction = D3DXVECTOR3(0.0f,0.0f, 1.0f);
	pLight->padding = 0.0f;
	m_pContext->Unmap(m_LightBuffer, 0);

	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pContext->PSSetShader(m_pPixelShader, nullptr, 0);
	m_pContext->PSSetSamplers(0, 1, &m_pSampleState);
	m_pContext->PSSetShaderResources(0, 1, &m_pShaderResourceView);
	m_pContext->IASetInputLayout(m_pInputLayout);
	m_pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	m_pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//draw all objects;
	m_pContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffers[0]);
	m_pContext->PSSetConstantBuffers(0, 1, &m_LightBuffer);
	Objects[0]->UpdateWorld();
	m_pContext->DrawIndexed(m_indexCount, 0, 0);
	
	return;
}

void CD3D11::DrawScene()
{
	if (m_bVsync_enabled == true)
	{
		m_pSwapChain->Present(1, 0);
	}
	else
	{
		m_pSwapChain->Present(0, 0);
	}
}
