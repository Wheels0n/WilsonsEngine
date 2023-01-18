#ifndef SETTINGS_H
#define SETTINGS_H

#include"Fps.h"
#include"camera.h"

class CSettings
{
public:
	CSettings() = default;
	CSettings(const CSettings&) = default;
	~CSettings() = default;

	void Init(CCamera*);
	void Draw();
private:
	CFps m_CFps;
	CCamera* m_pCCam;
};
#endif // !SETTINGS_H

