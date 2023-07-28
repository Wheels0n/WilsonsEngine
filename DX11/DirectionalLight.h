#pragma once
#include"Light.h"
namespace wilson
{	
	struct DirLightProperty
	{
		DirectX::XMVECTOR ambient;
		DirectX::XMVECTOR diffuse;
		DirectX::XMVECTOR specular;
		DirectX::XMFLOAT3 position;
		float pad;
	};

	class DirectionalLight : public Light
	{
	public:

		void UpdateViewMat();
		void UpdateProjMat();
		inline DirectX::XMMATRIX* GetLightSpaceMat()
		{
			return &m_lightSpaceMat;
		}
		inline DirectX::XMMATRIX* GetLitViewMat()
		{
			return &m_viewMat;
		}
		inline DirectX::XMMATRIX* GetLitProjMat()
		{
			return &m_projMat;
		}
		inline DirLightProperty* GetProperty()
		{
			return &m_dirLightProperty;
		}
		bool Init(ID3D11Device*);
		void UpdateProperty();
		ELIGHT_TYPE GetType() { return ELIGHT_TYPE::DIR; };

		DirectionalLight()=default;
		DirectionalLight(const DirectionalLight&) = default;
		~DirectionalLight() { Light::~Light(); };
	private:
		DirLightProperty m_dirLightProperty;

		DirectX::XMMATRIX m_lightSpaceMat;
		DirectX::XMMATRIX m_viewMat;
		DirectX::XMMATRIX m_projMat;
	};
}