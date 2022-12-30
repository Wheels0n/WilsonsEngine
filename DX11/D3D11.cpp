#include "D3D11.h"

CD3D11::CD3D11()
{
	m_pSwapChain = nullptr;
	m_pDevice = nullptr;
	m_pContext = nullptr;
	m_pRenderTargetView = nullptr;
	m_pDepthStencilBuffer = nullptr;
	m_pDefualtDDS = nullptr;
	m_pMirroMarkDDS = nullptr;
	m_pDrawReflectionDDS = nullptr;
	m_pDepthStencilView = nullptr;
	m_pRasterstate = nullptr;
	m_pRasterStateCC = nullptr;

	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
	m_pInputLayout = nullptr;
	m_pSampleState = nullptr;
	m_pLightBuffer = nullptr;

	m_pVertexBuffers = nullptr;
	m_pIndexBuffers = nullptr;
	m_pShaderResourceViews = nullptr;

	m_pNoRenderTargetWritesBS = nullptr;
	m_pTransparentBS = nullptr;
}

CD3D11::~CD3D11()
{
}

bool CD3D11::Init(int screenWidth, int screenHeight, bool bVsync, HWND hWnd, bool bFullscreen,
	float fScreenDepth, float fScreenNear)
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
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
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
;

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
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	//Set projectionMatrix, viewMatrix;



	//Set vertexData, indexData
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_INPUT_ELEMENT_DESC vertexDesc[3];
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	ID3DBlob* pVsBlob, * pPsBlob, * pErrorBlob;


	result = LoadFile(L"./Models/sphere/Sphere.obj");
	if (result == false)
	{
		return false;
	}

	result = LoadFile(L"./Models/wall/wall.obj");
	if (result == false)
	{
		return false;
	}

	result = LoadFile(L"./Models/tile/tile.obj");
	if (result == false)
	{
		return false;
	}

	m_pVertexBuffers = new ID3D11Buffer*[m_objectCount];

	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	for (int i = 0; i < m_objectCount; ++i)
	{  
		vertexData.pSysMem = m_pVertices[i];
		vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCounts[i];
		hr = m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffers[i]);
		if (FAILED(hr))
		{
			return false;
		}

	}
	//describe our indice
	m_pIndexBuffers = new ID3D11Buffer*[m_objectCount];

	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;


	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	for (int i = 0; i < m_objectCount; ++i)
	{   
		indexData.pSysMem = m_pIndices[i];
		indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCounts[i];
		hr = m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffers[i]);
		if (FAILED(hr))
		{
			return false;
		}
	}
	

	//Load VS to VS variable
	hr = D3DX10CompileFromFile(L"VS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pVsBlob, &pErrorBlob, nullptr);
	if (FAILED(hr))
	{
		return false;
	}
	m_pDevice->CreateVertexShader(pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), nullptr, &m_pVertexShader);
	m_pContext->VSSetShader(m_pVertexShader, nullptr, 0);


	hr = D3DX10CompileFromFile(L"PS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pPsBlob, &pErrorBlob, nullptr);
	if (FAILED(hr))
	{
		return false;
	}
	m_pDevice->CreatePixelShader(pPsBlob->GetBufferPointer(), pPsBlob->GetBufferSize(), nullptr, &m_pPixelShader);
	m_pContext->PSSetShader(m_pPixelShader, nullptr, 0);



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
	m_pDevice->CreateInputLayout(vertexDesc, sizeof(vertexDesc) / sizeof(vertexDesc[0]), pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &m_pInputLayout);
	m_pContext->IASetInputLayout(m_pInputLayout);



	m_pShaderResourceViews = new ID3D11ShaderResourceView*[4];
	//당분간 모델당 1텍스쳐
	hr = D3DX11CreateShaderResourceViewFromFileW(m_pDevice, L"./Models/sphere/Sphere.png", nullptr, nullptr, &m_pShaderResourceViews[m_texCount++], nullptr);
	if (FAILED(hr))
	{
		return false;
	}


	hr = D3DX11CreateShaderResourceViewFromFileW(m_pDevice, L"./Textures/brick01.dds", nullptr, nullptr, &m_pShaderResourceViews[m_texCount++], nullptr);
	if (FAILED(hr))
	{
		return false;
	}

	hr = D3DX11CreateShaderResourceViewFromFileW(m_pDevice, L"./Textures/checkboard.dds", nullptr, nullptr, &m_pShaderResourceViews[m_texCount++], nullptr);
	if (FAILED(hr))
	{
		return false;
	}

	hr = D3DX11CreateShaderResourceViewFromFileW(m_pDevice, L"./Textures/ice.dds", nullptr, nullptr, &m_pShaderResourceViews[m_texCount++], nullptr);
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
	m_pContext->PSSetSamplers(0, 1, &m_pSampleState);


	//Setting Constant Buffers
	D3D11_BUFFER_DESC lightCbd;
	lightCbd.Usage = D3D11_USAGE_DYNAMIC;
	lightCbd.ByteWidth = sizeof(Light);
	lightCbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightCbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightCbd.MiscFlags = 0;
	lightCbd.StructureByteStride = 0;
	m_pDevice->CreateBuffer(&lightCbd, nullptr, &m_pLightBuffer);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	Light* pLight;
	m_pContext->Map(m_pLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	pLight = reinterpret_cast<Light*>(mappedResource.pData);
	pLight->specular = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
	pLight->diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
	pLight->direction = D3DXVECTOR3(-1.0f, -2.0f, 1.0f);
	pLight->ambient = D3DXVECTOR4(0.1f, 0.1f, 0.1f, 1.0f);
	pLight->specPow = 32.0f;
	m_pContext->Unmap(m_pLightBuffer, 0);
	m_pContext->PSSetConstantBuffers(0, 1, &m_pLightBuffer);

		Objects[0] = new CObject(m_pDevice, m_pContext, &m_projectionMatrix, &m_viewMatrix);
		Objects[0]->Init(D3DXVECTOR3(0.0f, -0.5f, -1.0f), D3DXVECTOR3(0.3f, 0.3f, 0.3f));
		m_pMatrixBuffers[0] = Objects[0]->getMB();
		m_pCamBuffer = Objects[0]->getCB();
		m_pContext->VSSetConstantBuffers(1, 1, &m_pCamBuffer);

		Objects[1] = new CObject(m_pDevice, m_pContext, &m_projectionMatrix, &m_viewMatrix);
		Objects[1]->Init(D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(1.0f, 1.0f, 1.0f));
		m_pMatrixBuffers[1] = Objects[1]->getMB();

		Objects[2] = new CObject(m_pDevice, m_pContext, &m_projectionMatrix, &m_viewMatrix);
		Objects[2]->Init(D3DXVECTOR3(0.0f, -1.0f, -1.0f), D3DXVECTOR3(1.0f, 1.0f, 1.0f));
		m_pMatrixBuffers[2] = Objects[2]->getMB();
		
		//mirror
		Objects[3] = new CObject(m_pDevice, m_pContext, &m_projectionMatrix, &m_viewMatrix);
		Objects[3]->Init(D3DXVECTOR3(0.0f, 0.0f, -0.1f), D3DXVECTOR3(0.7f, 0.7f, 0.7f));
		m_pMatrixBuffers[3] = Objects[3]->getMB();
		//reflected
		Objects[4] = new CObject(m_pDevice, m_pContext, &m_projectionMatrix, &m_viewMatrix);
		Objects[4]->Init(D3DXVECTOR3(0.0f, -0.5f, 1.0f), D3DXVECTOR3(0.3f, 0.3f, 0.3f));
		m_pMatrixBuffers[4] = Objects[4]->getMB();

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

	if (m_pRasterStateCC != nullptr)
	{
		m_pRasterStateCC->Release();
		m_pRasterStateCC = nullptr;
	}

	if (m_pDepthStencilView != nullptr)
	{
		m_pDepthStencilView->Release();
		m_pDepthStencilView = nullptr;
	}

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


	if (m_pSampleState != nullptr)
	{
		m_pSampleState->Release();
		m_pSampleState = nullptr;
	}

	for (int i = m_objectCount - 1; i >= 0; --i)
	{
		if (m_pVertexBuffers != nullptr)
		{
			m_pVertexBuffers[i]->Release();
		}

		if (m_pIndexBuffers != nullptr)
		{
			m_pIndexBuffers[i]->Release();
		}
	}


	if (m_pShaderResourceViews != nullptr)
	{
		for (int i = 0; i < 4; ++i)
		{
			m_pShaderResourceViews[i]->Release();
		}
		delete[] m_pShaderResourceViews;
		m_pShaderResourceViews = nullptr;
	}

	if (m_pVertexBuffers != nullptr)
	{  
		delete[] m_pVertexBuffers;
		m_pVertexBuffers = nullptr;
	}

	if (m_pIndexBuffers != nullptr)
	{
		delete[] m_pIndexBuffers;
		m_pIndexBuffers = nullptr;
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
	return;
}


void CD3D11::UpdateScene()
{  
	//clear views
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int stride;
	unsigned int offset;
	float blendV[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	
	m_pContext->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
 	m_pContext->OMSetBlendState(nullptr, blendV, 0xf);
	m_pContext->OMSetDepthStencilState(m_pDefualtDDS, 1);
	stride = sizeof(VertexType);
	offset = 0;

	for (int i = 0; i < m_objectCount; ++i)
	{   
		if (i == 1)
		{
			continue;
		}
		m_pContext->PSSetShaderResources(0, 1, &m_pShaderResourceViews[i]);
		m_pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffers[i], &stride, &offset);
		m_pContext->IASetIndexBuffer(m_pIndexBuffers[i], DXGI_FORMAT_R32_UINT, 0);
		m_pContext->VSSetConstantBuffers(0, 1, &m_pMatrixBuffers[i]);
		Objects[i]->UpdateWorld(dx, dy, dz, dphi);
		m_pContext->DrawIndexed(m_indexCounts[i], 0, 0);
	}

	Objects[3]->UpdateWorld(dx, dy, dz, dphi);
	Objects[4]->UpdateWorld(dx, dy, dz, dphi);
	//dx = 0.0f, dy = 0.0f, dz = 0.0f, dphi = 0.0f;
	//0 sphere 1 wall 2 tile 3 mirror 4 reflection 
	m_pContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pContext->OMSetBlendState(m_pNoRenderTargetWritesBS, blendV, 0xf);
	m_pContext->OMSetDepthStencilState(m_pMirroMarkDDS, 1);
	m_pContext->PSSetShaderResources(0, 1, &m_pShaderResourceViews[2]);
	m_pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffers[1], &stride, &offset);
	m_pContext->IASetIndexBuffer(m_pIndexBuffers[1], DXGI_FORMAT_R32_UINT, 0);
	m_pContext->VSSetConstantBuffers(0, 1, &m_pMatrixBuffers[3]);
	m_pContext->DrawIndexed(m_indexCounts[1], 0, 0);
	
	m_pContext->OMSetBlendState(m_pTransparentBS, blendV, 0xf);
	m_pContext->OMSetDepthStencilState(m_pDrawReflectionDDS, 1);
	m_pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffers[0], &stride, &offset);
	m_pContext->IASetIndexBuffer(m_pIndexBuffers[0], DXGI_FORMAT_R32_UINT, 0);
	m_pContext->PSSetShaderResources(0, 1, &m_pShaderResourceViews[0]);
	m_pContext->VSSetConstantBuffers(0, 1, &m_pMatrixBuffers[4]);
	m_pContext->DrawIndexed(m_indexCounts[0], 0, 0);

	m_pContext->OMSetDepthStencilState(m_pDefualtDDS, 1);
	m_pContext->PSSetShaderResources(0, 1, &m_pShaderResourceViews[3]);
	m_pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffers[1], &stride, &offset);
	m_pContext->IASetIndexBuffer(m_pIndexBuffers[1], DXGI_FORMAT_R32_UINT, 0);
	m_pContext->VSSetConstantBuffers(0, 1, &m_pMatrixBuffers[3]);
	m_pContext->DrawIndexed(m_indexCounts[1], 0, 0);

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