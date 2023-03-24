#include "Light.h"

namespace wilson
{
	Light::Light(ID3D11Device* device, ID3D11DeviceContext* context)
	{
		m_pDevice = device;
		m_pContext = context;
		m_pLightBuffer = nullptr;
		m_pMaterialBuffer = nullptr;
	}
	Light::~Light()
	{
		if (m_pLightBuffer != nullptr)
		{
			m_pLightBuffer->Release();
			m_pLightBuffer = nullptr;
		}

		if (m_pMaterialBuffer != nullptr)
		{
			m_pMaterialBuffer->Release();
			m_pMaterialBuffer = nullptr;
		}
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
		m_dirLight.direction = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

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


		D3D11_BUFFER_DESC materialCBD;
		materialCBD.Usage = D3D11_USAGE_DYNAMIC;
		materialCBD.ByteWidth = sizeof(Material);
		materialCBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		materialCBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		materialCBD.MiscFlags = 0;
		materialCBD.StructureByteStride = 0;
		m_pDevice->CreateBuffer(&materialCBD, nullptr, &m_pMaterialBuffer);
		
		m_material.ambient = DirectX::XMVectorSet(0.0215f, 0.1714f, 0.0215f, 1.0f);
		m_material.diffuse = DirectX::XMVectorSet(0.07568f, 0.06124, 0.07568f, 1.0f);
		m_material.specular = DirectX::XMVectorSet(0.633f, 0.727811f, 0.633f, 25.0f);

		return true;
	}
	void Light::Update()
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		LightBuffer* pLightBuffer;
		Material* pMaterial;

		m_pContext->Map(m_pLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		pLightBuffer = reinterpret_cast<LightBuffer*>(mappedResource.pData);
		pLightBuffer->dirLight = m_dirLight;
		pLightBuffer->spotLight = m_spotLight;
		pLightBuffer->pointLight = m_pointLight;
		m_pContext->Unmap(m_pLightBuffer, 0);
		m_pContext->PSSetConstantBuffers(0, 1, &m_pLightBuffer);

		m_pContext->Map(m_pMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		pMaterial = reinterpret_cast<Material*>(mappedResource.pData);
		pMaterial->ambient = m_material.ambient;
		pMaterial->diffuse = m_material.diffuse;
		pMaterial->specular = m_material.specular;
		pMaterial->reflect = m_material.reflect;
		m_pContext->Unmap(m_pMaterialBuffer, 0);
		m_pContext->PSSetConstantBuffers(1, 1, &m_pMaterialBuffer);

	}
}