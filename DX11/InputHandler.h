#ifndef _INPUTHANDLER_H
#define _INPUTHANDLER_H

class CInputHandler
{
public:
	CInputHandler();
	CInputHandler(CInputHandler&) = delete;
	~CInputHandler();

	void Init();

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);

	bool IsKeyDown(unsigned int);

private:
	bool m_bKeys[256];
};

#endif
