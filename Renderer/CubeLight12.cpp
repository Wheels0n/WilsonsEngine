#include<DirectXMath.h>
#include "CubeLight12.h"
#include "HeapManager.h"
namespace wilson
{
    constexpr float g_near = 1.0f;
    constexpr float g_far = 150.0f;
    constexpr float g_ratio = 1.0f;
    constexpr float g_FOV = DirectX::XMConvertToRadians(90.0f);
    DirectX::XMMATRIX CubeLight12::g_perspectiveMat =
        DirectX::XMMatrixPerspectiveFovLH(g_FOV, g_ratio, g_near, g_far);
    std::vector<DirectX::XMVECTOR> CubeLight12::g_upVectors = {
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    };
    std::vector<DirectX::XMVECTOR> CubeLight12::g_dirVectors = {
        DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(-1.0f,0.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f,-1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
    };


    void CubeLight12::UpdateProperty()
    {
        m_CubeLightProperty.ambient = m_ambient;
        m_CubeLightProperty.attenuation = m_attenuation;
        m_CubeLightProperty.diffuse = m_diffuse;
        m_CubeLightProperty.position = m_position;
        m_CubeLightProperty.range = m_range;
        m_CubeLightProperty.specular = m_specular;
    }
    void CubeLight12::CreateShadowMatrices()
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
   
    void CubeLight12::UploadShadowMatrices(ID3D12GraphicsCommandList* pCommandlist)
    {
        std::vector<DirectX::XMMATRIX> matrices(6);
        for (int i = 0; i < 6; ++i)
        {
            matrices[i] = m_cubeMats[i];
        }

        memcpy(m_pMatricesCbBegin, &matrices[0], sizeof(DirectX::XMMATRIX) * 6);
        pCommandlist->SetGraphicsRootDescriptorTable(eCubeShadowRP::eCubeShadow_eGsMat , m_matriceCBV);
        return;
    }
   
    void CubeLight12::UploadLightPos(ID3D12GraphicsCommandList* pCommandlist)
    {
        DirectX::XMVECTOR pos = DirectX::XMVectorSet(m_position.x, m_position.y, m_position.z, g_far);

        memcpy(m_pPosCbBegin, &pos, sizeof(DirectX::XMVECTOR));
        pCommandlist->SetGraphicsRootDescriptorTable(eCubeShadowRP::eCubeShadow_ePsLightPos, m_posCBV);
        return;
    }


    CubeLight12::CubeLight12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandlist, HeapManager* pHeapManager, UINT idx)
        :Light12(idx)
    {
        m_pMatricesCbBegin = nullptr;
        m_pPosCbBegin = nullptr;

        UINT cbSize = sizeof(DirectX::XMMATRIX) * 7;
        m_pMatricesCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
        m_matriceCBV = pHeapManager->GetCBV(cbSize, pDevice);
                
        cbSize = sizeof(DirectX::XMVECTOR);
        m_pPosCbBegin= pHeapManager->GetCbMappedPtr(cbSize);
        m_posCBV = pHeapManager->GetCBV(cbSize, pDevice);
 
        m_cubeMats.resize(6);
        CreateShadowMatrices();
    }

    CubeLight12::~CubeLight12()
    {
        Light12::~Light12();
    }
}
