#include "D3D.h"

CD3D::CD3D()
{
	m_pDevice = nullptr;
	m_pCommandQueue = nullptr;
	m_pSwapChain = nullptr;
	m_pRenderTargetViewHeap = nullptr;
	m_pBackBufferRenderTarget[0] = nullptr;
	m_pBackBufferRenderTarget[1] = nullptr;
	m_pCommandAllocator = nullptr;
	m_pCommandList = nullptr;
	m_pPipelinState = nullptr;
	m_pRootSignature = nullptr;
	m_pFence = nullptr;
	m_pFenceEvent = nullptr;
	m_CModel = nullptr;
	debugController = nullptr;
}

CD3D::CD3D(const CD3D&)
{
}

CD3D::~CD3D()
{
}

bool CD3D::Initialize(int iScreenHeight, int iScreenWidth, HWND hWnd, bool bVsync, bool bFullscreen)
{   
#if defined(DEBUG) || defined(_DEBUG)
{
		D3D12GetDebugInterface(__uuidof(ID3D12Debug), reinterpret_cast<void**>(&debugController));
		debugController->EnableDebugLayer();
}
#endif

	HRESULT hr;
	D3D_FEATURE_LEVEL featureLevel;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	IDXGIFactory4* pFactory;
	IDXGIAdapter* pAdapter;
	IDXGIOutput* pAdapterOutput;
	unsigned int uiNumModes, i, uiNumerator, uiDenominator, uiRenderTargetViewDescriptorSize;
	unsigned long long ullStringLength;
	DXGI_MODE_DESC* pDisplayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int iError;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	IDXGISwapChain* pSwapChain;
	D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc;
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle;

	m_bVsync_enabled = bVsync;


	featureLevel = D3D_FEATURE_LEVEL_12_0;

	hr = D3D12CreateDevice(nullptr, featureLevel, __uuidof(ID3D12Device), reinterpret_cast<void**>(&m_pDevice));
	if (FAILED(hr))
	{
		MessageBox(hWnd, L"Could not create a DirectX 12.0 device. The default video card does not support DirectX 12.0",
			L"DirectX Device Failure", MB_OK);
		return false;
	}

	ZeroMemory(&commandQueueDesc, sizeof(commandQueueDesc));

	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;

	hr = m_pDevice->CreateCommandQueue(&commandQueueDesc, __uuidof(ID3D12CommandQueue), reinterpret_cast<void**>(&m_pCommandQueue));
	if (FAILED(hr))
	{
		return false;
	}

	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)&pFactory);
	if (FAILED(hr))
	{
		return false;
	}

	hr = pFactory->EnumAdapters(0, &pAdapter);
	if (FAILED(hr))
	{
		return false;
	}

	hr = pAdapter->EnumOutputs(0, &pAdapterOutput);
	if (FAILED(hr))
	{
		return false;
	}

	hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &uiNumModes, nullptr);
	if (FAILED(hr))
	{
		return false;
	}

	pDisplayModeList = new DXGI_MODE_DESC[uiNumModes];
	if (pDisplayModeList == nullptr)
	{
		return false;
	}

	hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &uiNumModes, pDisplayModeList);
	if (FAILED(hr))
	{
		return false;
	}

	for (i=0; i < uiNumModes; ++i)
	{
		if (pDisplayModeList[i].Height == static_cast<unsigned int>(iScreenHeight))
		{
			if (pDisplayModeList[i].Width == static_cast<unsigned int>(iScreenWidth))
			{
				uiNumerator = pDisplayModeList[i].RefreshRate.Numerator;
				uiDenominator = pDisplayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	hr = pAdapter->GetDesc(&adapterDesc);
	if (FAILED(hr))
	{
		return false;
	}

	m_iVideoCardMemory = static_cast<int>(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	iError = wcstombs_s(&ullStringLength, m_chVideoCardDescription, 128, adapterDesc.Description, 128);
	if (iError != 0)
	{
		return false;
	}

	delete[] pDisplayModeList;
	pDisplayModeList = nullptr;

	pAdapterOutput->Release();
	pAdapterOutput = nullptr;

	pAdapter->Release();
	pAdapter = nullptr;

	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 2;

	swapChainDesc.BufferDesc.Height = iScreenHeight;
	swapChainDesc.BufferDesc.Width = iScreenWidth;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	swapChainDesc.OutputWindow = hWnd;

	if (bFullscreen == true)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	if (m_bVsync_enabled == true)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = uiNumerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = uiDenominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swapChainDesc.Flags = 0;

	hr = pFactory->CreateSwapChain(m_pCommandQueue, &swapChainDesc, &pSwapChain);
	if (FAILED(hr))
	{
		return false;
	}

	hr = pSwapChain->QueryInterface(__uuidof(IDXGISwapChain3), reinterpret_cast<void**>(&m_pSwapChain));
	if (FAILED(hr))
	{
		return false;
	}

	pSwapChain = nullptr;

	pFactory->Release();
	pFactory = nullptr;

	ZeroMemory(&renderTargetViewHeapDesc, sizeof(renderTargetViewHeapDesc));

	renderTargetViewHeapDesc.NumDescriptors = 2;
	renderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	renderTargetViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = m_pDevice->CreateDescriptorHeap(&renderTargetViewHeapDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&m_pRenderTargetViewHeap));
	if (FAILED(hr))
	{
		return false;
	}

	renderTargetViewHandle = m_pRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	uiRenderTargetViewDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_pBackBufferRenderTarget[0]));
	if (FAILED(hr))
	{
		return false;
	}
	m_pDevice->CreateRenderTargetView(m_pBackBufferRenderTarget[0], nullptr, renderTargetViewHandle);
	renderTargetViewHandle.ptr += uiRenderTargetViewDescriptorSize;

	hr = m_pSwapChain->GetBuffer(1, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_pBackBufferRenderTarget[1]));
	if (FAILED(hr))
	{
		return false;
	}

	m_pDevice->CreateRenderTargetView(m_pBackBufferRenderTarget[1], nullptr, renderTargetViewHandle);

	m_uiBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(&m_pCommandAllocator));
	if (FAILED(hr))
	{
		return false;
	}

	hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator, nullptr,
		__uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(&m_pCommandList));
	if (FAILED(hr))
	{
		return false;
	}

	hr = m_pCommandList->Close();
	if (FAILED(hr))
	{
		return false;
	}

	hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), reinterpret_cast<void**>(&m_pFence));
	if (FAILED(hr))
	{
		return false;
	}

	m_pFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (m_pFenceEvent == nullptr)
	{
		return false;
	}

	m_ullFenceValue = 1;

	m_CModel = new CModel;
	if (m_CModel == nullptr)
	{
		return false;
	}

	return true;
}

void CD3D::Shutdown()
{
	int error;

	if (m_pSwapChain != nullptr)
	{
		m_pSwapChain->SetFullscreenState(false, nullptr);
	}

	error = CloseHandle(m_pFenceEvent);
	if (error != 0)
	{
	}

	if (m_pFence != nullptr)
	{
		m_pFence->Release();
		m_pFence = nullptr;
	}

	if (m_pPipelinState != nullptr)
	{
		m_pPipelinState->Release();
		m_pPipelinState = nullptr;
	}

	if (m_pCommandList != nullptr)
	{
		m_pCommandList->Release();
		m_pCommandList = nullptr;
	}

	if (m_pCommandAllocator != nullptr)
	{
		m_pCommandAllocator -> Release();
		m_pCommandAllocator = nullptr;
	}

	if (m_pBackBufferRenderTarget[0] != nullptr)
	{
		m_pBackBufferRenderTarget[0]->Release();
		m_pBackBufferRenderTarget[0] = nullptr;
	}

	if (m_pBackBufferRenderTarget[1] != nullptr)
	{
		m_pBackBufferRenderTarget[1]->Release();
		m_pBackBufferRenderTarget[1] = nullptr;
	}

	if (m_pRenderTargetViewHeap != nullptr)
	{
		m_pRenderTargetViewHeap->Release();
		m_pRenderTargetViewHeap = nullptr;
	}

	if (m_pSwapChain != nullptr)
	{
		m_pSwapChain->Release();
		m_pSwapChain = nullptr;
	}

	if (m_pCommandQueue != nullptr)
	{
		m_pCommandQueue->Release();
		m_pCommandQueue = nullptr;
	}

	if (m_pDevice != nullptr)
	{
		m_pDevice->Release();
		m_pDevice = nullptr;
	}
	
	if (m_CModel != nullptr)
	{
		delete m_CModel;
		m_CModel = nullptr;
	}
	return;
}

bool CD3D::Render()
{
    
	HRESULT hr;
	D3D12_RESOURCE_BARRIER barrier;
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle;
	unsigned int uiRenderTargetViewDescriptorSize;
	float color[4] = { 0, };
	ID3D12CommandList* ppCommandLists[1];
	unsigned long long ullFenceToWaitFor;

	hr = m_pCommandAllocator->Reset();
	if (FAILED(hr))
	{
		return false;
	}

	hr = m_pCommandList->Reset(m_pCommandAllocator, m_pPipelinState);
	if (FAILED(hr))
	{
		return false;
	}

	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pBackBufferRenderTarget[m_uiBufferIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_pCommandList->ResourceBarrier(1, &barrier);

	renderTargetViewHandle = m_pRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	uiRenderTargetViewDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	if (m_uiBufferIndex == 1)
	{
		renderTargetViewHandle.ptr += uiRenderTargetViewDescriptorSize;
	}

	m_pCommandList->OMSetRenderTargets(1, &renderTargetViewHandle, false, nullptr);
	m_pCommandList->ClearRenderTargetView(renderTargetViewHandle, color, 0, nullptr);

	m_CModel->Initialize(m_pDevice);
	m_CModel->Render(m_pDevice, m_pCommandList);

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_pCommandList->ResourceBarrier(1, &barrier);
	hr = m_pCommandList->Close();
	if (FAILED(hr))
	{
		return false;
	}

	ppCommandLists[0] = m_pCommandList;
    
	m_pCommandQueue->ExecuteCommandLists(1, ppCommandLists);

	if (m_bVsync_enabled == true)
	{
		hr = m_pSwapChain->Present(1, 0);
		if (FAILED(hr))
		{
			return false;
		}
	}
	else
	{
		hr = m_pSwapChain->Present(0, 0);
		if (FAILED(hr))
		{
			return false;
		}
	}

	ullFenceToWaitFor = m_ullFenceValue;
	hr = m_pCommandQueue->Signal(m_pFence, ullFenceToWaitFor);
	if (FAILED(hr))
	{
		return false;
	}

	m_ullFenceValue++;

	if (m_pFence->GetCompletedValue() < ullFenceToWaitFor)
	{
		hr = m_pFence->SetEventOnCompletion(ullFenceToWaitFor, m_pFenceEvent);
		if (FAILED(hr))
		{
			return false;
		}
		WaitForSingleObject(m_pFenceEvent, INFINITE);
	}

	m_uiBufferIndex == 0 ? m_uiBufferIndex = 1 : m_uiBufferIndex = 0;
	return true;
}
