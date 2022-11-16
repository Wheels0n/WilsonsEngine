#ifndef _INPUTHANDLER_H_
#define _INPUTHANDLER_H_
// xinput 사용예정
class CInputHandler
{
public:
	CInputHandler();
	CInputHandler(const CInputHandler&);
	~CInputHandler();

	void Initialize();

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);

	bool IsKeyDown(unsigned int);

private:
	bool m_bKeys[256];
};

#endif