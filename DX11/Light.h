#ifndef LIGHT_H
#define LIGHT_H

#include <dxgi.h>
#include <d3d11.h>
#include <DirectXMath.h>

namespace wilson
{   
	struct DirectionalLight
	{
		DirectX::XMVECTOR specular;
		DirectX::XMVECTOR ambient;
		DirectX::XMVECTOR diffuse;
		DirectX::XMFLOAT3 direction;
		float pad;
	};
	struct PointLight
	{
		DirectX::XMVECTOR ambient;
		DirectX::XMVECTOR diffuse;
		DirectX::XMVECTOR specular;

		DirectX::XMFLOAT3 position;
		float range;

		DirectX::XMFLOAT3 attenuation;
		float pad;
	};
	struct SpotLight
	{
		DirectX::XMVECTOR ambient;
		DirectX::XMVECTOR diffuse;
		DirectX::XMVECTOR specular;

		DirectX::XMFLOAT3 position;
		float range;

		DirectX::XMFLOAT3 direction;
		float spot;

		DirectX::XMFLOAT3 attenuation;
		float pad;
	};
	struct LightBuffer
	{
		DirectionalLight  dirLight;
		PointLight        pointLight;
		SpotLight		  spotLight;
	};

	class Light
	{
	public:
		bool Init();
		void Update();

		inline DirectionalLight GetDirLight()
		{
			return m_dirLight;
		}
		inline PointLight       GetPointLight()
		{
			return m_pointLight;
		}
		inline SpotLight        GetSpotLight()
		{
			return m_spotLight;
		}
		inline void             SetDirLight(DirectionalLight dirLight)
		{
			m_dirLight = dirLight;
		}
		inline void             SetPointLight(PointLight pointLight)
		{
			m_pointLight = pointLight;
		}
		inline void             SetSpotLight(SpotLight spotLight)
		{
			m_spotLight = spotLight;
		}

		Light(ID3D11Device* pDevice, ID3D11DeviceContext* context);
		~Light();
	private:

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pContext;
		ID3D11Buffer* m_pLightBuffer;

		DirectionalLight m_dirLight;
		PointLight       m_pointLight;
		SpotLight        m_spotLight;
	};
}
#endif 
