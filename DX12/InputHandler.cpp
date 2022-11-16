#include "InputHandler.h"

CInputHandler::CInputHandler()
{
}

CInputHandler::CInputHandler(const CInputHandler&)
{
}

CInputHandler::~CInputHandler()
{
}

void CInputHandler::Initialize()
{
	int i;
	for (i = 0; i < 256; i++)
	{
		m_bKeys[i] = false;
	}

	return;
}

void CInputHandler::KeyDown(unsigned int input)
{
	m_bKeys[input] = true;
	return;
}

void CInputHandler::KeyUp(unsigned int input)
{
	m_bKeys[input] = false;
	return;
}

bool CInputHandler::IsKeyDown(unsigned int input)
{
	return m_bKeys[input];
}
