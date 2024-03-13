#include<DirectXMath.h>
#include "CubeLight.h"
namespace wilson
{   
    constexpr float g_near = 1.0f;
    constexpr float g_far =150.0f;
    constexpr float g_ratio = 1.0f;
    constexpr float g_FOV = DirectX::XMConvertToRadians(90.0f);
    DirectX::XMMATRIX CubeLight::g_perspectiveMat =
        DirectX::XMMatrixPerspectiveFovLH(g_FOV, g_ratio, g_near, g_far);
    std::vector<DirectX::XMVECTOR> CubeLight::g_upVectors = {
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    };
    std::vector<DirectX::XMVECTOR> CubeLight::g_dirVectors = {
        DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), 
        DirectX::XMVectorSet(-1.0f,0.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f,-1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
    };

  
    void CubeLight::UpdateProperty()
    {
        m_CubeLightProperty.ambient = m_ambient;
        m_CubeLightProperty.attenuation = m_attenuation;
        m_CubeLightProperty.diffuse = m_diffuse;
        m_CubeLightProperty.position = m_position;
        m_CubeLightProperty.range = m_range;
        m_CubeLightProperty.specular = m_specular;
    }
    void CubeLight::CreateShadowMatrices()
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
    void CubeLight::UploadShadowMatrices(ID3D11DeviceContext* pContext)
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DirectX::XMMATRIX* pMatrix;
        HRESULT hr = pContext->Map(m_pMatricesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        assert(SUCCEEDED(hr));

        pMatrix = reinterpret_cast<DirectX::XMMATRIX*> (mappedResource.pData);
        for (int i = 0; i < 6; ++i)
        {
            pMatrix[i] = m_cubeMats[i];
        }
        pContext->Unmap(m_pMatricesBuffer, 0);
        pContext->GSSetConstantBuffers(0, 1, &m_pMatricesBuffer);
        
    }
    void CubeLight::UploadLightPos(ID3D11DeviceContext* pContext)
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DirectX::XMVECTOR* pV;
        HRESULT hr = pContext->Map(m_pPosBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        assert(SUCCEEDED(hr));

        pV = reinterpret_cast<DirectX::XMVECTOR*> (mappedResource.pData);
        *pV = DirectX::XMVectorSet(m_position.x, m_position.y, m_position.z, g_far);
 

        pContext->Unmap(m_pPosBuffer, 0);
        pContext->PSSetConstantBuffers(0, 1, &m_pPosBuffer);
    }

    CubeLight::CubeLight(ID3D11Device* pDevice, UINT idx)
        :Light(idx)
    {
        m_pMatricesBuffer = nullptr;
        m_pPosBuffer = nullptr;

        m_cubeMats.resize(6);
        D3D11_BUFFER_DESC cbufferDesc = {};
        cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbufferDesc.ByteWidth = sizeof(CubeLightProperty);
        cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbufferDesc.MiscFlags = 0;
        cbufferDesc.StructureByteStride = 0;

        HRESULT result = pDevice->CreateBuffer(&cbufferDesc, nullptr, &m_pLightBuffer);
        if (FAILED(result))
        {   
            OutputDebugStringA("CubeLight::m_pLightBuffer::CreateBufferFailed");
        }
        m_pLightBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            sizeof("CubeLight::m_pLightBuffer") - 1, "CubeLight::m_pLightBuffer");

        cbufferDesc.ByteWidth = sizeof(DirectX::XMVECTOR);
        result = pDevice->CreateBuffer(&cbufferDesc, nullptr, &m_pPosBuffer);
        if (FAILED(result))
        {
            OutputDebugStringA("CubeLight::m_pPosBuffer::CreateBufferFailed");
        }
        m_pPosBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            sizeof("CubeLight::m_pPosBuffer") - 1, "CubeLight::m_pPosBuffer");

       
        m_attenuation = DirectX::XMFLOAT3(0.0f, 0.1f, 0.0f);
        m_range = 25.0f;

        cbufferDesc.ByteWidth = sizeof(DirectX::XMMATRIX) * 7;
        result = pDevice->CreateBuffer(&cbufferDesc, nullptr, &m_pMatricesBuffer);
        if (FAILED(result))
        {
            OutputDebugStringA("CubeLight::m_pMatricesBuffer::CreateBufferFailed");
        }
        m_pMatricesBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            sizeof("CubeLight::m_pMatricesBuffer") - 1, "CubeLight::m_pMatricesBuffer");
        CreateShadowMatrices();

    }

    CubeLight::~CubeLight()
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
