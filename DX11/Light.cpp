#include "Light.h"

namespace wilson
{
	DirectX::XMMATRIX* Light::GetLightSpaceMat()
	{	
		m_lightSpaceMat = DirectX::XMMatrixMultiply(m_viewMat, m_projMat);
		return &m_lightSpaceMat;
	}
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
	void Light::UpdateViewMat(Camera* pCam)
	{	
		DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR target = DirectX::XMVectorZero();
		DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&m_position);
		m_viewMat = DirectX::XMMatrixLookAtLH(pos,
			target, up);
	}
	void Light::UpdateProjMat(Camera* pCam)
	{	
		//오브젝트마다 aabb?
		m_projMat = DirectX::XMMatrixOrthographicLH(100.0f, 100.0f, 0.1f, 100.0f); 
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