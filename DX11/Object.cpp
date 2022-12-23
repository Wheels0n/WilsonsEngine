#include "Object.h"

CObject::CObject(ID3D11Device* device, ID3D11DeviceContext* context, D3DXMATRIX* projectionMatrix, D3DXMATRIX* viewMatrix)
{
 	m_pContext = context;
	m_pDevice = device;
	m_pMatrixBuffer = nullptr;
	m_projectionMatrix = *projectionMatrix;
	m_viewMatrix = *viewMatrix;
	m_pCamBuffer = nullptr;

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
	D3D11_BUFFER_DESC mbDesc, cbDesc;

	mbDesc.Usage = D3D11_USAGE_DYNAMIC;
	mbDesc.ByteWidth = sizeof(ConstantBufferType);
	mbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mbDesc.MiscFlags = 0;
	mbDesc.StructureByteStride = 0;
	hr = m_pDevice->CreateBuffer(&mbDesc, 0, &m_pMatrixBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.ByteWidth = sizeof(camBuffer);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;
	hr = m_pDevice->CreateBuffer(&cbDesc, 0, &m_pCamBuffer);
	if (FAILED(hr))
	{
		return false;
	}


	D3DXMatrixTranspose(&m_projectionMatrix, &m_projectionMatrix);
	return true;
}

void CObject::ShutDown()
{
	if (m_pMatrixBuffer != nullptr)
	{
		m_pMatrixBuffer->Release();
		m_pMatrixBuffer = nullptr;
	}

	if (m_pCamBuffer != nullptr)
	{
		m_pCamBuffer->Release();
		m_pCamBuffer = nullptr;
	}
}

void CObject::UpdateWorld()
{  
	float dt = (float)D3DX_PI * 0.005f;
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ConstantBufferType* pMatrices;
	D3DXMATRIX rotationMatrix;
	
	yaw += dt;
	if (yaw > 360.0f)
	{
		yaw -= 360.0f;
	}
	D3DXMatrixRotationY(&m_worldMatrix, yaw);
	D3DXMatrixTranslation(&m_viewMatrix, x, y, z);
	D3DXMatrixRotationY(&rotationMatrix, dphi);
	D3DXMatrixMultiply(&m_viewMatrix, &m_viewMatrix, &rotationMatrix);
	//ROW-MAJOR(CPU) TO COL-MAJOR(GPU)
	D3DXMatrixTranspose(&m_worldMatrix, &m_worldMatrix);
	D3DXMatrixTranspose(&m_viewMatrix, &m_viewMatrix);
	//write CPU data into GPU mem;
	hr = m_pContext->Map(m_pMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		return ;
	}
	pMatrices = reinterpret_cast<ConstantBufferType*>(mappedResource.pData);
	pMatrices->world = m_worldMatrix;
	pMatrices->view = m_viewMatrix;
	pMatrices->projection = m_projectionMatrix;
	m_pContext->Unmap(m_pMatrixBuffer, 0);
	//D3DXMatrixIdentity(&m_worldMatrix);

	camBuffer* pCamBuffer;
	hr = m_pContext->Map(m_pCamBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		return;
	}
	pCamBuffer = reinterpret_cast<camBuffer*>(mappedResource.pData);
	pCamBuffer->camPos = D3DXVECTOR4(m_viewMatrix._11, m_viewMatrix._12, m_viewMatrix._13, m_viewMatrix._14);
	m_pContext->Unmap(m_pCamBuffer, 0);
	return;
}
