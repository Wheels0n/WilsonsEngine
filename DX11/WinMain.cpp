#include "Engine.h"
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

	return 0;
}
