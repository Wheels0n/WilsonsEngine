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

	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
	m_pInputLayout = nullptr;
	m_pSampleState = nullptr;
	m_pLightBuffer = nullptr;

	m_pVertexBuffers = nullptr;
	m_pIndexBuffers = nullptr;
	m_pShaderResourceViews = nullptr;
	m_objectCount = 0;
	m_texCount = 0;

	m_texture = nullptr;
	m_plte = nullptr;
	m_pngData = nullptr;
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
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	//Set projectionMatrix, viewMatrix;
	fFOV = static_cast<float>(D3DX_PI) / 4.0f;
	fScreenAspect = screenWidth / static_cast<float>(screenHeight);
	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fFOV, fScreenAspect, fScreenNear, fScreenDepth);

	D3DXVECTOR3 m_vPos = { 0.0f, 0.0f, -1.0f };  //Translation
	D3DXVECTOR3 m_vLookat = { 0.0f, 0.0f, 0.0f };//camera look-at target
	D3DXVECTOR3 m_vUp = { 0.0f, 1.0f, 0.0f };   //which axis is upward
	D3DXMatrixLookAtLH(&m_viewMatrix, &m_vPos, &m_vLookat, &m_vUp);


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



	m_pShaderResourceViews = new ID3D11ShaderResourceView*[m_objectCount];
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
	pLight->direction = D3DXVECTOR3(0.0f, -1.0f, 1.0f);
	pLight->ambient = D3DXVECTOR4(0.1f, 0.1f, 0.1f, 1.0f);
	pLight->specPow = 32.0f;
	m_pContext->Unmap(m_pLightBuffer, 0);
	m_pContext->PSSetConstantBuffers(0, 1, &m_pLightBuffer);

		Objects[0] = new CObject(m_pDevice, m_pContext, &m_projectionMatrix, &m_viewMatrix);
		Objects[0]->Init(D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.6f, 0.6f, 0.6f));
		m_pMatrixBuffers[0] = Objects[0]->getMB();
		m_pCamBuffer = Objects[0]->getCB();
		m_pContext->VSSetConstantBuffers(1, 1, &m_pCamBuffer);


		Objects[1] = new CObject(m_pDevice, m_pContext, &m_projectionMatrix, &m_viewMatrix);
		Objects[1]->Init(D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(1.0f, 1.0f, 1.0f));
		m_pMatrixBuffers[1] = Objects[1]->getMB();
		//m_pCamBuffer = Objects[1]->getCB();

		Objects[2] = new CObject(m_pDevice, m_pContext, &m_projectionMatrix, &m_viewMatrix);
		Objects[2]->Init(D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(1.0f, 1.0f, 1.0f));
		m_pMatrixBuffers[2] = Objects[2]->getMB();
		//m_pCamBuffer = Objects[2]->getCB();

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

	for (int i = m_objectCount-1; i >=0; --i)
	{
		if (m_pVertexBuffers!= nullptr)
		{
			m_pVertexBuffers[i]->Release();
		}

		if (m_pIndexBuffers != nullptr)
		{
			m_pIndexBuffers[i]->Release();
		}

		if (m_pVertexCoordinates[i] != nullptr)
		{
			delete[] m_pVertexCoordinates[i];
			m_pVertexCoordinates.pop_back();
		}

		if (m_pTexCoordinates[i] != nullptr)
		{
			delete[] m_pTexCoordinates[i];
			m_pTexCoordinates.pop_back();
		}

		if (m_pNormalVectors[i] != nullptr)
		{
			delete[] m_pNormalVectors[i];
			m_pNormalVectors.pop_back();
		}

		if (m_pVertices[i] != nullptr)
		{
			delete[] m_pVertices[i];
			m_pVertices.pop_back();
		}

		if (m_pIndices[i] != nullptr)
		{
			delete[] m_pIndices[i];
			m_pIndices.pop_back();
		}

		if (m_pShaderResourceViews != nullptr)
		{
			m_pShaderResourceViews[i]->Release();
		}
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

	if (m_pShaderResourceViews != nullptr)
	{   
		delete[] m_pShaderResourceViews;
		m_pShaderResourceViews = nullptr;
	}

	if (m_vertexCounts.size() > 0)
	{
		m_vertexCounts.clear();
		m_vertexCounts.shrink_to_fit();
	}

	if (m_indexCounts.size() > 0)
	{
		m_indexCounts.clear();
		m_indexCounts.shrink_to_fit();
	}

	if (m_vertexCoordCounts.size() > 0)
	{
		m_vertexCoordCounts.clear();
		m_vertexCoordCounts.shrink_to_fit();
	}

	if (m_texCoordCounts.size() > 0)
	{
		m_texCoordCounts.clear();
		m_texCoordCounts.shrink_to_fit();
	}

	if (m_normalVectorCounts.size() > 0)
	{
		m_normalVectorCounts.clear();
		m_normalVectorCounts.shrink_to_fit();
	}

	if (m_plte != nullptr)
	{
		delete[] m_plte;
		m_plte = nullptr;
	}

	if (m_pngData != nullptr)
	{
		delete[] m_pngData;
		m_pngData = nullptr;
	}

	if (m_texture != nullptr)
	{
		m_texture->Release();
		m_texture = nullptr;
	}

	return;
}


void CD3D11::UpdateScene()
{  
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currtime));

	//clear views
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int stride;
	unsigned int offset;
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	
	m_pContext->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	stride = sizeof(VertexType);
	offset = 0;

	for (int i = 0; i < m_objectCount; ++i)
	{
		m_pContext->PSSetShaderResources(0, 1, &m_pShaderResourceViews[i]);
		m_pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffers[i], &stride, &offset);
		m_pContext->IASetIndexBuffer(m_pIndexBuffers[i], DXGI_FORMAT_R32_UINT, 0);
		//draw all objects;
		m_pContext->VSSetConstantBuffers(0, 1, &m_pMatrixBuffers[i]);
		
		Objects[i]->UpdateWorld(dx, dy, dz , dphi);
		m_pContext->DrawIndexed(m_indexCounts[i], 0, 0);
	}
	dx = 0.0f, dy = 0.0f, dz = 0.0f, dphi = 0.0f;
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

bool CD3D11::LoadFile(LPCWSTR fileName)
{   
	std::ifstream fin;
	fin.open(fileName);

	if (fin.fail())
	{
		return false;
	}
	char ch;
	std::string line;
	
	m_vertexCoordCounts.reserve(m_objectCount + 1);
	m_vertexCoordCounts.push_back(0);
	m_texCoordCounts.reserve(m_objectCount + 1);
	m_texCoordCounts.push_back(0);
	m_normalVectorCounts.reserve(m_objectCount + 1);
	m_normalVectorCounts.push_back(0);
	m_vertexCounts.reserve(m_objectCount + 1);
	m_vertexCounts.push_back(0);
	
	while (!fin.eof())
	{   
		std::getline(fin, line, ' ');
		if (line.compare("v")==0)
		{
			++m_vertexCoordCounts[m_objectCount];
		}
		else if (line.compare("vt") == 0)
		{
			++m_texCoordCounts[m_objectCount];
		}

		else if (line.compare("vn") == 0)
		{
			++m_normalVectorCounts[m_objectCount];
		}

		else if (line.compare("f") == 0)
		{   
			fin.get(ch);
			++m_vertexCounts[m_objectCount];
			while (ch!='\n')
			{   
				if (ch == ' ')
				{
					++m_vertexCounts[m_objectCount];
				}
				fin.get(ch);
			
			}
			continue;
		}
		std::getline(fin,line);
	}
	fin.close();
	ch = ' ';

	m_pVertexCoordinates.reserve(m_objectCount + 1);
	m_pVertexCoordinates.push_back(nullptr);
	m_pVertexCoordinates[m_objectCount] = new D3DXVECTOR3[m_vertexCoordCounts[m_objectCount]];
	
	m_pTexCoordinates.reserve(m_objectCount + 1);
	m_pTexCoordinates.push_back(nullptr);
	m_pTexCoordinates[m_objectCount] = new D3DXVECTOR2[m_texCoordCounts[m_objectCount]];
	
	m_pNormalVectors.reserve(m_objectCount + 1);
	m_pNormalVectors.push_back(nullptr);
	m_pNormalVectors[m_objectCount] = new D3DXVECTOR3[m_normalVectorCounts[m_objectCount]];
	
	m_pVertices.reserve(m_objectCount + 1);
	m_pVertices.push_back(nullptr);
	m_pVertices[m_objectCount] = new VertexType[m_vertexCounts[m_objectCount]];
	
	m_pIndices.reserve(m_objectCount + 1);
	m_pIndices.push_back(nullptr);
	m_pIndices[m_objectCount] = new unsigned long[m_vertexCounts[m_objectCount]];

	m_indexCounts.reserve(m_objectCount + 1);
	m_indexCounts.push_back(0);


	ZeroMemory(m_pVertexCoordinates[m_objectCount], sizeof(D3DXVECTOR3) * m_vertexCoordCounts[m_objectCount]);
	ZeroMemory(m_pTexCoordinates[m_objectCount], sizeof(D3DXVECTOR2) * m_texCoordCounts[m_objectCount]);
	ZeroMemory(m_pNormalVectors[m_objectCount], sizeof(D3DXVECTOR3) * m_normalVectorCounts[m_objectCount]);
	ZeroMemory(m_pVertices[m_objectCount], sizeof(VertexType) * m_vertexCounts[m_objectCount]);

	fin.open(fileName);
	if (fin.fail())
	{
		return false;
	}

	char type;
	int vCnt = 0, vtCnt = 0, vnCnt = 0, i;
	while (!fin.eof())
	{
		fin.get(type);

		if (type=='v')
		{  
			fin.get(type);
			if (type == ' ')
			{
				fin >> m_pVertexCoordinates[m_objectCount][vCnt].x 
					>> m_pVertexCoordinates[m_objectCount][vCnt].y 
					>> m_pVertexCoordinates[m_objectCount][vCnt].z;
				m_pVertexCoordinates[m_objectCount][vCnt].z *= -1;
				++vCnt;
			}
			else if (type == 't')
			{
				fin >> m_pTexCoordinates[m_objectCount][vtCnt].x 
					>> m_pTexCoordinates[m_objectCount][vtCnt].y;
				m_pTexCoordinates[m_objectCount][vtCnt].y = 1 - m_pTexCoordinates[m_objectCount][vtCnt].y;
				++vtCnt;
			}
			else if (type == 'n')
			{
				fin >> m_pNormalVectors[m_objectCount][vnCnt].x 
					>> m_pNormalVectors[m_objectCount][vnCnt].y 
					>> m_pNormalVectors[m_objectCount][vnCnt].z;
				m_pNormalVectors[m_objectCount][vnCnt].z *= -1;
				++vnCnt;
			}
		}

		else if (type ==  'f')
		{   
			fin.get(type);
			if (type == ' ')
			{
				int v, vt, vn;

				while (!fin.fail())
				{
					fin >> v >> ch;
					fin >> vt >> ch;
					fin >> vn;
					if (!fin.fail())
					{   
						i = m_indexCounts[m_objectCount];
						m_pVertices[m_objectCount][i].position = m_pVertexCoordinates[m_objectCount][v-1];
						m_pVertices[m_objectCount][i].tex = m_pTexCoordinates[m_objectCount][vt-1];
						m_pVertices[m_objectCount][i].norm = m_pNormalVectors[m_objectCount][vn-1];
						//오른손좌표계에서 왼손좌표계로
						m_pIndices[m_objectCount][i] = m_vertexCounts[m_objectCount] - i-1;
						++m_indexCounts[m_objectCount];
					}

				}
				fin.clear();
			}
		}
	}

	fin.close();

	++m_objectCount;
	return true;
}

bool CD3D11::LoadPNG(LPCWSTR fileName, unsigned int* width, unsigned int* height)
{  
	std::ifstream fin;

	unsigned int len=0;
	unsigned char buffer[4];
	char data[38];
	char chunkType[5];
	char bitsPerPixel, colorType, compressionMethod, filterMethod;
	chunkType[4] = '\0';

	fin.open(fileName, std::ifstream::binary);
	if (fin.fail())
	{
		return false;
	}

	fin.seekg(8, std::ios::cur);//discard PNG signature;
	
	while (!fin.fail())
	{   
		fin.read(reinterpret_cast<char*>(buffer), 4);
		len = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];

		fin.read(chunkType, 4);
		if (strcmp("IHDR", chunkType) == 0)
		{
			fin.read(reinterpret_cast<char*>(buffer), 4);
			*width  = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
			fin.read(reinterpret_cast<char*>(buffer), 4);
			*height = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
			fin.read(&bitsPerPixel, 1);
			fin.read(&colorType, 1);
			fin.read(&compressionMethod, 1);
			fin.read(&filterMethod, 1);
			
			fin.seekg(len - 12, std::ios::cur);
		}

		else if (strcmp("PLTE", chunkType) == 0)
		{   
			m_plte = new char[len];
			fin.read(m_plte, len);
		}
		else if (strcmp("IDAT", chunkType) == 0)
		{  
			fin.seekg(2, std::ios::cur);
			m_pngData = new char[len];
			fin.read(m_pngData, len);
		}
		else if (strcmp("IEND", chunkType) == 0)
		{
			break;
		}
		else
		{
			fin.seekg(len, std::ios::cur);
		}
		fin.seekg(4, std::ios::cur); // discard CRC;
	}

	fin.close();
	return true;
}
