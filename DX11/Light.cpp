#include "Light.h"

namespace wilson
{
	Light::Light()
	{
		m_pLightBuffer = nullptr;
	}
	Light::~Light()
	{
		if (m_pLightBuffer != nullptr)
		{
			m_pLightBuffer->Release();
			m_pLightBuffer = nullptr;
		}
		
	}
	bool Light::Init(ID3D11Device* pDevice)
	{
		m_ambient = DirectX::XMVectorZero();
		m_diffuse = DirectX::XMVectorZero();
		m_specular = DirectX::XMVectorZero();
		m_position = DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f);
		return true;
	}

}