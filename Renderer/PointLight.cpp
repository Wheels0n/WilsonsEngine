#include<DirectXMath.h>
#include "PointLight.h"
namespace wilson
{   
    constexpr float g_near = 1.0f;
    constexpr float g_far =150.0f;
    constexpr float g_ratio = 1.0f;
    constexpr float g_FOV = DirectX::XMConvertToRadians(90.0f);
    DirectX::XMMATRIX PointLight::g_perspectiveMat =
        DirectX::XMMatrixPerspectiveFovLH(g_FOV, g_ratio, g_near, g_far);
    std::vector<DirectX::XMVECTOR> PointLight::g_upVectors = {
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    };
    std::vector<DirectX::XMVECTOR> PointLight::g_dirVectors = {
        DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), 
        DirectX::XMVectorSet(-1.0f,0.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f,-1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
    };

  
    void PointLight::UpdateProperty()
    {
        m_pointLightProperty.ambient = m_ambient;
        m_pointLightProperty.attenuation = m_attenuation;
        m_pointLightProperty.diffuse = m_diffuse;
        m_pointLightProperty.position = m_position;
        m_pointLightProperty.range = m_range;
        m_pointLightProperty.specular = m_specular;
    }
    void PointLight::CreateShadowMatrices()
    {   
        DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&m_position);
        
        //우,좌,상,하,앞,뒤 순
        for (int i = 0; i < 6; ++i)
        {
            DirectX::XMMATRIX viewMat =
                DirectX::XMMatrixLookAtLH(pos, DirectX::XMVectorAdd(pos, g_dirVectors[i]), g_upVectors[i]);
            m_cubeMats[i] = DirectX::XMMatrixMultiplyTranspose(viewMat, g_perspectiveMat);
        }
    }
    void PointLight::UploadShadowMatrices(ID3D11DeviceContext* pContext)
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DirectX::XMMATRIX* pMatrix;
        HRESULT hr = pContext->Map(m_pMatricesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (FAILED(hr))
        {   
            OutputDebugStringA("PointlLight::m_pMatriceBuffer::MapFailed");
            return;
        }

        pMatrix = reinterpret_cast<DirectX::XMMATRIX*> (mappedResource.pData);
        for (int i = 0; i < 6; ++i)
        {
            pMatrix[i] = m_cubeMats[i];
        }
        pContext->Unmap(m_pMatricesBuffer, 0);
        pContext->GSSetConstantBuffers(0, 1, &m_pMatricesBuffer);
        
    }
    void PointLight::UploadLightPos(ID3D11DeviceContext* pContext)
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DirectX::XMVECTOR* pV;
        HRESULT hr = pContext->Map(m_pPosBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (FAILED(hr))
        {   
            OutputDebugStringA("PointlLight::m_pPosBuffer::MapFailed");
            return;
        }

        pV = reinterpret_cast<DirectX::XMVECTOR*> (mappedResource.pData);
        *pV = DirectX::XMVectorSet(m_position.x, m_position.y, m_position.z, g_far);
 

        pContext->Unmap(m_pPosBuffer, 0);
        pContext->PSSetConstantBuffers(0, 1, &m_pPosBuffer);
    }

    PointLight::PointLight(ID3D11Device* pDevice, UINT idx)
        :Light(idx)
    {
        m_pMatricesBuffer = nullptr;
        m_pPosBuffer = nullptr;

        m_cubeMats.resize(6);
        D3D11_BUFFER_DESC cbufferDesc = {};
        cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbufferDesc.ByteWidth = sizeof(PointLightProperty);
        cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbufferDesc.MiscFlags = 0;
        cbufferDesc.StructureByteStride = 0;

        HRESULT result = pDevice->CreateBuffer(&cbufferDesc, nullptr, &m_pLightBuffer);
        if (FAILED(result))
        {   
            OutputDebugStringA("PointLight::m_pLightBuffer::CreateBufferFailed");
        }
        m_pLightBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            sizeof("PointLight::m_pLightBuffer") - 1, "PointLight::m_pLightBuffer");

        cbufferDesc.ByteWidth = sizeof(DirectX::XMVECTOR);
        result = pDevice->CreateBuffer(&cbufferDesc, nullptr, &m_pPosBuffer);
        if (FAILED(result))
        {
            OutputDebugStringA("PointLight::m_pPosBuffer::CreateBufferFailed");
        }
        m_pPosBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            sizeof("PointLight::m_pPosBuffer") - 1, "PointLight::m_pPosBuffer");

       
        m_attenuation = DirectX::XMFLOAT3(0.0f, 0.1f, 0.0f);
        m_range = 25.0f;

        cbufferDesc.ByteWidth = sizeof(DirectX::XMMATRIX) * 7;
        result = pDevice->CreateBuffer(&cbufferDesc, nullptr, &m_pMatricesBuffer);
        if (FAILED(result))
        {
            OutputDebugStringA("PointLight::m_pMatricesBuffer::CreateBufferFailed");
        }
        m_pMatricesBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            sizeof("PointLight::m_pMatricesBuffer") - 1, "PointLight::m_pMatricesBuffer");
        CreateShadowMatrices();

    }

    PointLight::~PointLight()
    {
        if (m_pMatricesBuffer != nullptr)
        {
            m_pMatricesBuffer->Release();
            m_pMatricesBuffer = nullptr;
        }

        if (m_pPosBuffer != nullptr)
        {
            m_pPosBuffer->Release();
            m_pPosBuffer = nullptr;
        }

        Light::~Light();
    }

}
