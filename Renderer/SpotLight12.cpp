#include "SpotLight12.h"
#include "HeapManager.h"
namespace wilson
{

    SpotLight12::SpotLight12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandlist, HeapManager* pHeapManager, UINT idx)
        :Light12(idx)
    {
       
    }
    void SpotLight12::UpdateViewMat()
    {
        DirectX::XMVECTOR dir = XMLoadFloat3(&m_direction);
        dir = DirectX::XMVector3Normalize(dir);

        DirectX::XMVECTOR pos = XMLoadFloat3(&m_position);
        DirectX::XMVECTOR target = DirectX::XMVectorAdd(pos, dir);
        DirectX::XMFLOAT3 target3;
        DirectX::XMStoreFloat3(&target3, target);

        m_viewMat = DirectX::XMMatrixLookAtLH(
            DirectX::XMLoadFloat3(&m_position),
            DirectX::XMLoadFloat3(&target3),
            DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        UpdateLitMat();
    }
    void SpotLight12::UpdateProjMat()
    {
        m_perspectiveMat = DirectX::XMMatrixPerspectiveFovLH(
            DirectX::XMConvertToRadians(m_outerCutoff * 2.0f), 1.0f, _S_NEAR, _S_FAR);
        UpdateLitMat();
    }

    void SpotLight12::UpdateLitMat()
    {
        m_lightSpaceMat = DirectX::XMMatrixMultiplyTranspose(m_viewMat, m_perspectiveMat);
    }

    void SpotLight12::UpdateProperty()
    {
        m_spotLightProperty.ambient = m_ambient;
        m_spotLightProperty.attenuation = m_attenuation;
        m_spotLightProperty.diffuse = m_diffuse;
        m_spotLightProperty.direction = m_direction;
        m_spotLightProperty.position = m_position;
        m_spotLightProperty.range = m_range;
        m_spotLightProperty.specular = m_specular;
        m_spotLightProperty.cutoff = DirectX::XMScalarCos(DirectX::XMConvertToRadians(m_cutoff));
        m_spotLightProperty.outerCutoff = DirectX::XMScalarCos(DirectX::XMConvertToRadians(m_outerCutoff));

    }

}