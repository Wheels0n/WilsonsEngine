#include "Engine.h"
#include<d3d11.h>

#ifdef _DEBUG

#include <dxgidebug.h>
DEFINE_GUID(DXGI_DEBUG_D3D11, 0x4b99317b, 0xac39, 0x4aa6, 0xbb, 0xb, 0xba, 0xa0, 0x47, 0x84, 0x79, 0x8f);

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

void D3DMemoryLeakCheck()
{
	HMODULE dxgidebugDLL = GetModuleHandleW(L"dxgidebug.dll");
	decltype(&DXGIGetDebugInterface) GetDebugInterface =
		reinterpret_cast<decltype(&DXGIGetDebugInterface)>(GetProcAddress(dxgidebugDLL, "DXGIGetDebugInterface"));

	IDXGIDebug* pDebug;
	GetDebugInterface(IID_PPV_ARGS(&pDebug));

	OutputDebugStringW(L"!!!D3D 메모리 누수 체크!!!\r\n");
	pDebug->ReportLiveObjects(DXGI_DEBUG_D3D11,DXGI_DEBUG_RLO_DETAIL);
	OutputDebugStringW(L"!!!반환되지 않은 IUnKnown 객체!!!\r\n");
	
	pDebug->Release();
}
#endif // _DEBUG

int CALLBACK WinMain(HINSTANCE hInstance,
	HINSTANCE hPreInstance,
	LPSTR lpCmdLine,
	int   nCmdShow)
{
	wilson::Engine* g_pEngine;
	bool g_bResult;

	g_pEngine = new wilson::Engine;
	if (g_pEngine == nullptr)
	{
		return 0;
	}

	g_bResult = g_pEngine->Init();
	if (g_bResult == true)
	{
		g_pEngine->Run();
	}

	g_pEngine->Shutdown();
	delete g_pEngine;
	g_pEngine = nullptr;

#ifdef _DEBUG
	D3DMemoryLeakCheck();
#endif // _DEBUG

	return 0;
}
