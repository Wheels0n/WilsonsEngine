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
	m_pSampleState = nullptr;
	m_LightBuffer = nullptr;

	m_pShaderResourceView = nullptr;
	m_texture = nullptr;

	verticeCoordinates = nullptr;
	texCoordinates = nullptr;
	normalVectors = nullptr;
	vertices = nullptr;

	m_vertexCount = 0;
	m_vertexCoordCount = 0;
	m_texCoordCount = 0;
	m_normalVectorCount = 0;
	m_indexCount = 0;

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

	//Set projectionMatrix, viewMatrix;
	fFOV = static_cast<float>(D3DX_PI) / 4.0f;
	fScreenAspect = screenWidth / static_cast<float>(screenHeight);
	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fFOV, fScreenAspect, fScreenNear, fScreenDepth);
	
	D3DXVECTOR3 m_vPos = { 0.0f,10.0f,-150.0f };  //Translation
	D3DXVECTOR3 m_vLookat = { 0.0f, 1.0f, 0.0f };//camera look-at target
	D3DXVECTOR3 m_vUp = { 0.0f, 1.0f, 0.0f };   //which axis is upward
	D3DXMatrixLookAtLH(&m_viewMatrix, &m_vPos, &m_vLookat, &m_vUp);


	//Set vertexData, indexData
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_INPUT_ELEMENT_DESC vertexDesc[3];
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	ID3DBlob* pVsBlob, * pPsBlob, * pErrorBlob;


	result = LoadFile(L"./teapot/teapot.obj");
	if (result == false)
	{
		return false;
	}


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
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_vertexCount;
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

	hr = D3DX11CreateShaderResourceViewFromFileW(m_pDevice, L"./teapot/teapot1.png", nullptr, nullptr, &m_pShaderResourceView, nullptr);
	if(FAILED(hr))
	{
		return false;
	}
	/*
	unsigned int textureWidth=0, textureHeight=0;

	result = LoadPNG(L"./teapot/teapot1.png", &textureWidth, &textureHeight);
	if (result==false)
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	hr = m_pDevice->CreateTexture2D(&textureDesc, nullptr, &m_texture);
	if (FAILED(hr))
	{
		return false;
	}

	unsigned int rowPitch = (textureWidth * 4) * sizeof(unsigned char); 
	m_pContext->UpdateSubresource(m_texture, 0, nullptr, m_pngData, rowPitch, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	hr = m_pDevice->CreateShaderResourceView(m_texture, &srvDesc, &m_pShaderResourceView);
	if (FAILED(hr))
	{
		return false;
	}
	*/

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

	if (verticeCoordinates != nullptr)
	{
		delete[] verticeCoordinates;
		verticeCoordinates = nullptr;
	}

	if (texCoordinates != nullptr)
	{
		delete[] texCoordinates;
		texCoordinates = nullptr;
	}

	if (normalVectors != nullptr)
	{
		delete[] normalVectors;
		normalVectors = nullptr;
	}

	if (vertices != nullptr)
	{
		delete[] vertices;
		vertices = nullptr;
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

	if (m_pShaderResourceView != nullptr)
	{
		m_pShaderResourceView->Release();
		m_pShaderResourceView = nullptr;
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
	
	Objects[0]->x = dx;
	Objects[0]->y = dy;
	Objects[0]->z = dz;
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
	while (!fin.eof())
	{   
		std::getline(fin, line, ' ');
		if (line.compare("v")==0)
		{
			++m_vertexCoordCount;
		}
		else if (line.compare("vt") == 0)
		{
			++m_texCoordCount;
		}

		else if (line.compare("vn") == 0)
		{
			++m_normalVectorCount;
		}

		else if (line.compare("f") == 0)
		{   
			fin.get(ch);
			++m_vertexCount;
			while (ch!='\n')
			{   
				if (ch == ' ')
				{
					++m_vertexCount;
				}
				fin.get(ch);
			
			}
			continue;
		}
		std::getline(fin,line);
	}
	fin.close();
	ch = ' ';

	verticeCoordinates = new D3DXVECTOR3[m_vertexCoordCount];
	texCoordinates = new D3DXVECTOR2[m_texCoordCount];
	normalVectors = new D3DXVECTOR3[m_normalVectorCount];
	vertices = new VertexType[m_vertexCount];
	indices = new unsigned long[m_vertexCount];

	ZeroMemory(verticeCoordinates, sizeof(D3DXVECTOR3) * m_vertexCoordCount);
	ZeroMemory(texCoordinates, sizeof(D3DXVECTOR2) * m_texCoordCount);
	ZeroMemory(normalVectors, sizeof(D3DXVECTOR3) * m_normalVectorCount);
	ZeroMemory(vertices, sizeof(VertexType) * m_vertexCount);

	fin.open(fileName);
	if (fin.fail())
	{
		return false;
	}

	char type;
	int vCnt = 0, vtCnt = 0, vnCnt = 0;
	while (!fin.eof())
	{
		fin.get(type);

		if (type=='v')
		{  
			fin.get(type);
			if (type == ' ')
			{
				fin >> verticeCoordinates[vCnt].x >> verticeCoordinates[vCnt].y >> verticeCoordinates[vCnt].z;
				verticeCoordinates[vCnt].z *= -1;
				++vCnt;
			}
			else if (type == 't')
			{
				fin >> texCoordinates[vtCnt].x >> texCoordinates[vtCnt].y;
				texCoordinates[vtCnt].y = 1 - texCoordinates[vtCnt].y;
				++vtCnt;
			}
			else if (type == 'n')
			{
				fin >> normalVectors[vnCnt].x >> normalVectors[vnCnt].y >> normalVectors[vnCnt].z;
				normalVectors[vnCnt].z *= -1;
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
						vertices[m_indexCount].position = verticeCoordinates[v-1];
						vertices[m_indexCount].tex = texCoordinates[vt-1];
						vertices[m_indexCount].norm = normalVectors[vn-1];

						indices[m_indexCount] = m_vertexCount - m_indexCount-1;
						++m_indexCount;
					}

				}
				fin.clear();
			}
		}
	}

	fin.close();
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
