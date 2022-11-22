#include "Object.h"

CObject::CObject(ID3D11Device* device, ID3D11DeviceContext* context, D3DXMATRIX* projectionMatrix, D3DXMATRIX* viewMatrix)
	:x(dist(rng)), y(dist(rng)), z(dist(rng)), pitch(dist(rng)), yaw(dist(rng)), roll(dist(rng))
{
 	m_pContext = context;
	m_pDevice = device;
	m_pConstantBuffer = nullptr;
	m_projectionMatrix = *projectionMatrix;
	m_viewMatrix = *viewMatrix;

}

CObject::CObject(const CObject&)
{
}

CObject::~CObject()
{
}

bool CObject::Init()
{  
	HRESULT hr;
	D3D11_BUFFER_DESC cbDesc;

	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.ByteWidth = sizeof(ConstantBufferType);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;
	hr = m_pDevice->CreateBuffer(&cbDesc, 0, &m_pConstantBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	D3DXMatrixIdentity(&m_worldMatrix);
	D3DXMatrixTranslation(&m_worldMatrix, x, y, z);

	return true;
}

void CObject::ShutDown()
{
	if (m_pConstantBuffer != nullptr)
	{
		m_pConstantBuffer->Release();
		m_pConstantBuffer = nullptr;
	}
}

void CObject::UpdateWorld()
{  
	float dt = 0.001f;
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ConstantBufferType* pMatrices;

	pitch += dt;
	yaw += dt;
	roll += dt;

	if (pitch > 360);
	{
		pitch = 0u;
	}
	if (yaw > 360);
	{
		yaw = 0u;
	}
	if (roll > 360);
	{
		roll = 0u;
	}
	D3DXMatrixRotationYawPitchRoll(&m_rotationMatrix, yaw, pitch, roll);
	D3DXMatrixMultiply(&m_worldMatrix, &m_worldMatrix, &m_rotationMatrix);
	//ROW-MAJOR(CPU) TO COL-MAJOR(GPU)
	D3DXMatrixTranspose(&m_worldMatrix, &m_worldMatrix);
	D3DXMatrixTranspose(&m_viewMatrix, &m_viewMatrix);
	D3DXMatrixTranspose(&m_projectionMatrix, &m_projectionMatrix);

	//write CPU data into GPU mem;
	hr = m_pContext->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		return ;
	}
	pMatrices = reinterpret_cast<ConstantBufferType*>(mappedResource.pData);
	pMatrices->world = m_worldMatrix;
	pMatrices->view = m_viewMatrix;
	pMatrices->projection = m_projectionMatrix;
	m_pContext->Unmap(m_pConstantBuffer, 0);

	return;
}
