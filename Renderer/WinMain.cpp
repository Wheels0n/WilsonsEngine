#pragma once
#include "Engine.h"

int CALLBACK WinMain(HINSTANCE hInstance,
	HINSTANCE hPreInstance,
	LPSTR lpCmdLine,
	int   nCmdShow)
{
	wilson::Engine* g_pEngine;
	bool g_bResult;

	g_pEngine = new wilson::Engine;

	g_pEngine->Run();

	delete g_pEngine;
	g_pEngine = nullptr;

	return 0;
}
