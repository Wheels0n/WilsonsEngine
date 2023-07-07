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
		void Init(ID3D11Device*, Camera*);
		void Draw();

		Settings() = default;
		Settings(const Settings&) = default;
		~Settings();
	private:
		ID3D11ShaderResourceView* m_icons[3];//dir, point, spot;

		FPS m_FPS;
		Light* m_pLight;
		Camera* m_pCam;
	};
}
#endif // !SETTINGS_H

