#ifndef _D3D_H_
#define _D3D_H_

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include "Model.h"
class CD3D
{
public:
	CD3D();
	CD3D(const CD3D&);
	~CD3D();

	bool Initialize(int, int, HWND, bool, bool);
	void Shutdown();

	bool Render();

private:
	bool m_bVsync_enabled;
	ID3D12Debug* debugController;
	ID3D12Device* m_pDevice;
	ID3D12CommandQueue* m_pCommandQueue;
	char m_chVideoCardDescription[128];
	int  m_iVideoCardMemory;
	IDXGISwapChain3* m_pSwapChain;
	ID3D12DescriptorHeap* m_pRenderTargetViewHeap;
	ID3D12Resource* m_pBackBufferRenderTarget[2];
	unsigned int m_uiBufferIndex;
	ID3D12CommandAllocator* m_pCommandAllocator;
	ID3D12GraphicsCommandList* m_pCommandList;
	ID3D12PipelineState* m_pPipelinState;
	ID3D12RootSignature* m_pRootSignature;
	ID3D12Fence* m_pFence;
	HANDLE m_pFenceEvent;
	unsigned long long m_ullFenceValue;
    Cmodel * m_CModel;
};

#endif