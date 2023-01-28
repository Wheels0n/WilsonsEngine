#ifndef SETTINGS_H
#define SETTINGS_H

#include"Fps.h"
#include"camera.h"
#include"Light.h"

class CSettings
{
public:
	CSettings() = default;
	CSettings(const CSettings&) = default;
	~CSettings() = default;

	void Init(CCamera*, CLight*);
	void Draw();
private:
	CFps m_CFps;
	CLight* m_pCLight;
	CCamera* m_pCCam;
};
#endif // !SETTINGS_H

