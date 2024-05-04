
#include "CubeLight11.h"
namespace wilson
{   
    constexpr float g_near = 1.0f;
    constexpr float g_far =150.0f;
    constexpr float g_ratio = 1.0f;
    constexpr float g_FOV = DirectX::XMConvertToRadians(90.0f);
    DirectX::XMMATRIX CubeLight11::g_perspectiveMat =
        DirectX::XMMatrixPerspectiveFovLH(g_FOV, g_ratio, g_near, g_far);
    std::vector<DirectX::XMVECTOR> CubeLight11::g_upVectors = {
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    };
    std::vector<DirectX::XMVECTOR> CubeLight11::g_dirVectors = {
        DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), 
        DirectX::XMVectorSet(-1.0f,0.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f,-1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
    };

  
    void CubeLight11::UpdateProperty()
    {
        m_cubeLightProperty.ambient = m_ambient;
        m_cubeLightProperty.attenuation = m_attenuation;
        m_cubeLightProperty.diffuse = m_diffuse;
        m_cubeLightProperty.position = m_position;
        m_cubeLightProperty.range = m_range;
        m_cubeLightProperty.specular = m_specular;
    }
    void CubeLight11::CreateShadowMatrices()
    {   
        DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&m_position);
        
        //��,��,��,��,��,�� ��
        for (int i = 0; i < 6; ++i)
        {
            DirectX::XMMATRIX viewMat =
                DirectX::XMMatrixLookAtLH(pos, DirectX::XMVectorAdd(pos, g_dirVectors[i]), g_upVectors[i]);
            m_cubeMats[i] = DirectX::XMMatrixMultiplyTranspose(viewMat, g_perspectiveMat);
        }
    }
    void CubeLight11::UploadShadowMatrices(ID3D11DeviceContext* const pContext)
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DirectX::XMMATRIX* pMatrix;
        HRESULT hr = pContext->Map(m_pMatricesCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        assert(SUCCEEDED(hr));

        pMatrix = reinterpret_cast<DirectX::XMMATRIX*> (mappedResource.pData);
        for (int i = 0; i < 6; ++i)
        {
            pMatrix[i] = m_cubeMats[i];
        }
        pContext->Unmap(m_pMatricesCb.Get(), 0);
        pContext->GSSetConstantBuffers(0, 1, m_pMatricesCb.GetAddressOf());
        
    }
    void CubeLight11::UploadLightPos(ID3D11DeviceContext* const pContext)
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DirectX::XMVECTOR* pV;
        HRESULT hr = pContext->Map(m_pPosCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        assert(SUCCEEDED(hr));

        pV = reinterpret_cast<DirectX::XMVECTOR*> (mappedResource.pData);
        *pV = DirectX::XMVectorSet(m_position.x, m_position.y, m_position.z, g_far);
 

        pContext->Unmap(m_pPosCb.Get(), 0);
        pContext->PSSetConstantBuffers(0, 1, m_pPosCb.GetAddressOf());
    }

    CubeLight11::CubeLight11(ID3D11Device* const pDevice , const UINT idx)
        :Light11(idx)
    {
        m_cubeMats.resize(6);
        D3D11_BUFFER_DESC cbufferDesc = {};
        cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbufferDesc.ByteWidth = sizeof(CubeLightProperty);
        cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbufferDesc.MiscFlags = 0;
        cbufferDesc.StructureByteStride = 0;

        HRESULT result = pDevice->CreateBuffer(&cbufferDesc, nullptr, m_pLightCb.GetAddressOf());
        if (FAILED(result))
        {   
            OutputDebugStringA("CubeLight11::m_pLightCb::CreateBufferFailed");
        }
        m_pLightCb->SetPrivateData(WKPDID_D3DDebugObjectName,
            sizeof("CubeLight11::m_pLightCb") - 1, "CubeLight11::m_pLightCb");

        cbufferDesc.ByteWidth = sizeof(DirectX::XMVECTOR);
        result = pDevice->CreateBuffer(&cbufferDesc, nullptr, m_pPosCb.GetAddressOf());
        if (FAILED(result))
        {
            OutputDebugStringA("CubeLight11::m_pPosCb::CreateBufferFailed");
        }
        m_pPosCb->SetPrivateData(WKPDID_D3DDebugObjectName,
            sizeof("CubeLight11::m_pPosCb") - 1, "CubeLight11::m_pPosCb");

       
        m_attenuation = DirectX::XMFLOAT3(0.0f, 0.1f, 0.0f);
        m_range = 25.0f;

        cbufferDesc.ByteWidth = sizeof(DirectX::XMMATRIX) * 7;
        result = pDevice->CreateBuffer(&cbufferDesc, nullptr, m_pMatricesCb.GetAddressOf());
        if (FAILED(result))
        {
            OutputDebugStringA("CubeLight11::m_pMatricesCb::CreateBufferFailed");
        }
        m_pMatricesCb->SetPrivateData(WKPDID_D3DDebugObjectName,
            sizeof("CubeLight11::m_pMatricesCb") - 1, "CubeLight11::m_pMatricesCb");
        CreateShadowMatrices();

    }

    CubeLight11::~CubeLight11()
    {
        Light11::~Light11();
    }

}
