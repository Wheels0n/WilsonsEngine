#pragma once
#include"Light.h"
namespace wilson
{	
	struct DirLightProperty
	{
		DirectX::XMVECTOR specular;
		DirectX::XMVECTOR ambient;
		DirectX::XMVECTOR diffuse;
		DirectX::XMFLOAT3 position;
		float pad;
	};

	class DirectionalLight : public Light
	{
	public:
		inline DirLightProperty* GetProperty()
		{
			return &m_dirLightProperty;
		}
		bool Init(ID3D11Device*);
		void UpdateProperty();
		ELIGHT_TYPE GetType() { return ELIGHT_TYPE::DIR; };

		DirectionalLight() = default;
		DirectionalLight(const DirectionalLight&) = default;
		~DirectionalLight() { Light::~Light(); };
	private:
		DirLightProperty m_dirLightProperty;
	};
}