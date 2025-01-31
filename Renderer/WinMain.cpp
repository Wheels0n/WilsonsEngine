#pragma once
#include "Engine.h"

using namespace wilson;
int CALLBACK WinMain(HINSTANCE hInstance,
	HINSTANCE hPreInstance,
	LPSTR lpCmdLine,
	int   nCmdShow)
{
	unique_ptr<Engine> pEngine = std::make_unique<Engine>();
	pEngine->Run();

	return 0;
}
