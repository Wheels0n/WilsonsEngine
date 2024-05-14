
#include "CubeLight12.h"
#include "HeapManager.h"
namespace wilson
{
    constexpr float g_near = 1.0f;
    constexpr float g_far = 1500.0f;
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
        m_cubeLightProperty.ambient = m_ambient;
        m_cubeLightProperty.attenuation = m_attenuation;
        m_cubeLightProperty.diffuse = m_diffuse;
        m_cubeLightProperty.position = m_position;
        m_cubeLightProperty.range = m_range;
        m_cubeLightProperty.specular = m_specular;
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
   
    void CubeLight12::UploadShadowMatrices(ID3D12GraphicsCommandList* const pCommandlist)
    {
        std::vector<DirectX::XMMATRIX> matrices(6);
        for (int i = 0; i < 6; ++i)
        {
            matrices[i] = m_cubeMats[i];
        }

        memcpy(m_pMatricesCbBegin, &matrices[0], sizeof(DirectX::XMMATRIX) * 6);
        pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(eCubeShadowRP::gsMat) , m_matriceCbv);
        return;
    }
   
    void CubeLight12::UploadLightPos(ID3D12GraphicsCommandList* const pCommandlist)
    {
        DirectX::XMVECTOR pos = DirectX::XMVectorSet(m_position.x, m_position.y, m_position.z, g_far);

        memcpy(m_pPosCbBegin, &pos, sizeof(DirectX::XMVECTOR));
        pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(eCubeShadowRP::psLightPos), m_posCbv);
        return;
    }


    CubeLight12::CubeLight12(ID3D12Device* const pDevice, ID3D12GraphicsCommandList* const pCommandlist, HeapManager* const pHeapManager, const UINT idx)
        :Light12(idx)
    {
        m_pMatricesCbBegin = nullptr;
        m_pPosCbBegin = nullptr;

        UINT cbSize = sizeof(DirectX::XMMATRIX) * 7;
        m_pMatricesCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
        m_matriceCbv = pHeapManager->GetCbv(cbSize, pDevice);
                
        cbSize = sizeof(DirectX::XMVECTOR);
        m_pPosCbBegin= pHeapManager->GetCbMappedPtr(cbSize);
        m_posCbv = pHeapManager->GetCbv(cbSize, pDevice);
 
        m_cubeMats.resize(6);
        CreateShadowMatrices();
    }

    CubeLight12::~CubeLight12()
    {
        Light12::~Light12();
    }
}
