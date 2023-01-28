#include "Light.h"

CLight::CLight(ID3D11Device* device, ID3D11DeviceContext* context)
{   
	m_pDevice = device;
	m_pContext = context;
	m_pLightBuffer = nullptr;
}

CLight::~CLight()
{
	if (m_pLightBuffer != nullptr)
	{
		m_pLightBuffer->Release();
		m_pLightBuffer = nullptr;
	}
}

bool CLight::Init()
{   
	D3D11_BUFFER_DESC lightCbd;
	lightCbd.Usage = D3D11_USAGE_DYNAMIC;
	lightCbd.ByteWidth = sizeof(Light);
	lightCbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightCbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightCbd.MiscFlags = 0;
	lightCbd.StructureByteStride = 0;
	m_pDevice->CreateBuffer(&lightCbd, nullptr, &m_pLightBuffer);

	return true;
}

XMVECTOR* CLight::GetSpecular()
{
	return &m_vSpecular;
}

XMVECTOR* CLight::GetAmbient()
{
	return &m_vAmbient;
}

XMVECTOR* CLight::GetDiffuse()
{
	return &m_vDiffuse;
}

XMVECTOR* CLight::GetDirection()
{
	return &m_vDirection;
}

float* CLight::GetSpecPow()
{
	return &m_fSpecPow;
}

void CLight::SetSpecular(XMVECTOR spec)
{
	m_vSpecular = spec;
}

void CLight::SetAmbient(XMVECTOR ambi)
{
	m_vAmbient = ambi;
}

void CLight::SetDiffuse(XMVECTOR diff)
{
	m_vDiffuse = diff;
}

void CLight::SetDirection(XMVECTOR dir)
{
	m_vDirection = dir;
}

void CLight::SetSpecPow(float pow)
{
	m_fSpecPow = pow;
}

void CLight::Update()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	Light* pLight;

	m_pContext->Map(m_pLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	pLight = reinterpret_cast<Light*>(mappedResource.pData);
	pLight->specular = m_vSpecular;
	pLight->diffuse = m_vDiffuse;
	pLight->direction = m_vDirection;
	pLight->ambient = m_vAmbient;
	pLight->specPow = m_fSpecPow;
	m_pContext->Unmap(m_pLightBuffer, 0);

	m_pContext->PSSetConstantBuffers(0, 1, &m_pLightBuffer);
}
