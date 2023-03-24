#ifndef SETTINGS_H
#define SETTINGS_H

#include"FPS.h"
#include"Camera.h"
#include"Light.h"

namespace wilson
{
	class Settings
	{
	public:
		void Init(Camera*, Light*);
		void Draw();

		Settings() = default;
		Settings(const Settings&) = default;
		~Settings() = default;
	private:
		FPS m_CFps;
		Light* m_pLight;
		Camera* m_pCam;
	};
}
#endif // !SETTINGS_H

