#include "Light.h"

namespace wilson
{
	Light::Light()
	{
		m_ambient = m_ambient = DirectX::XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f);
		m_diffuse = m_ambient;
		m_specular = m_ambient;
		m_position = DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f);
		m_direction = DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f);;
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
		
		return true;
	}

}