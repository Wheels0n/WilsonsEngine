#include "Light.h"

namespace wilson
{
	DirectX::XMMATRIX* Light::GetLightSpaceMat()
	{	
		m_lightSpaceMat = DirectX::XMMatrixMultiply(m_viewMat, m_projMat);
		return &m_lightSpaceMat;
	}
	Light::Light(ID3D11Device* pDevice, ID3D11DeviceContext* context)
	{
		m_pDevice = pDevice;
		m_pContext = context;
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
		DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&m_dirLight.position);
		m_viewMat = DirectX::XMMatrixLookAtLH(pos,
			target, up);
	}
	void Light::UpdateProjMat(Camera* pCam)
	{	
		//오브젝트마다 aabb?
		m_projMat = DirectX::XMMatrixOrthographicLH(100.0f, 100.0f, 0.1f, 100.0f); 
	}
	bool Light::Init()
	{
		D3D11_BUFFER_DESC lightCBD;
		lightCBD.Usage = D3D11_USAGE_DYNAMIC;
		lightCBD.ByteWidth = sizeof(LightBuffer);
		lightCBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		lightCBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		lightCBD.MiscFlags = 0;
		lightCBD.StructureByteStride = 0;
		m_pDevice->CreateBuffer(&lightCBD, nullptr, &m_pLightBuffer);

		m_dirLight.ambient = DirectX::XMVectorSet(0.2f, 0.2f, 0.2f, 1.0f);
		m_dirLight.diffuse = DirectX::XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f);
		m_dirLight.specular = DirectX::XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f);
		m_dirLight.position = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

		m_pointLight.ambient = DirectX::XMVectorSet(0.3f, 0.3f, 0.3f, 1.0f);
		m_pointLight.diffuse = DirectX::XMVectorSet(0.7f, 0.7f, 0.7f, 1.0f);
		m_pointLight.specular = DirectX::XMVectorSet(0.7f, 0.7f, 0.7f, 1.0f);
		m_pointLight.attenuation = DirectX::XMFLOAT3(0.0f, 0.1f, 0.0f);
		m_pointLight.range = 25.0f;

		m_spotLight.ambient = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		m_spotLight.diffuse = DirectX::XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f);
		m_spotLight.specular = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		m_spotLight.attenuation = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_spotLight.spot = 96.0f;
		m_spotLight.range = 10000.0f;


		return true;
	}
	void Light::Update()
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		LightBuffer* pLightBuffer;

		m_pContext->Map(m_pLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		pLightBuffer = reinterpret_cast<LightBuffer*>(mappedResource.pData);
		pLightBuffer->dirLight = m_dirLight;
		pLightBuffer->spotLight = m_spotLight;
		pLightBuffer->pointLight = m_pointLight;
		m_pContext->Unmap(m_pLightBuffer, 0);
		m_pContext->PSSetConstantBuffers(0, 1, &m_pLightBuffer);

	}
}