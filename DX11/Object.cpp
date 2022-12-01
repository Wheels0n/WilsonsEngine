#include "Object.h"

CObject::CObject(ID3D11Device* device, ID3D11DeviceContext* context, D3DXMATRIX* projectionMatrix, D3DXMATRIX* viewMatrix)
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
	float dt = 1.0f/75.0*3.1415f;
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ConstantBufferType* pMatrices;

	
	yaw += dt;
	
	D3DXMatrixTranslation(&m_worldMatrix, 0.0, 0.0f, 0.0f);
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
	//D3DXMatrixIdentity(&m_worldMatrix);
	return;
}
