#include<cmath>

#include"DirectionalLight12.h"
#include"HeapManager.h"
namespace wilson
{
 
    DirectionalLight12::DirectionalLight12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandlist, HeapManager* pHeapManager, 
        UINT idx, Camera12* pCam)
        :Light12(idx)
    {
        m_pCam = pCam;
        m_lightSpaceMat.resize(pCam->GetCascadeLevels().size());
        UpdateLightSpaceMatrices();

        m_pMatricesCbBegin = nullptr;

        UINT cbSize = sizeof(DirectX::XMMATRIX) * _CASCADE_LEVELS;
        m_pMatricesCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
        m_matriceCBV = pHeapManager->GetCBV(cbSize, pDevice);

    }
    DirectionalLight12::~DirectionalLight12()
    {
        Light12::~Light12();
    }

    void DirectionalLight12::UpdateProperty()
    {
        m_dirLightProperty.ambient = m_ambient;
        m_dirLightProperty.diffuse = m_diffuse;
        m_dirLightProperty.specular = m_specular;
        m_dirLightProperty.direction = m_direction;
    }


    void DirectionalLight12::UploadShadowMatrices(ID3D12GraphicsCommandList* pCommandlist)
    {
        std::vector<DirectX::XMMATRIX> matrices(m_lightSpaceMat.size());
        for (int i = 0; i < m_lightSpaceMat.size(); ++i)
        {
            matrices[i] = m_lightSpaceMat[i];
        }

        memcpy(m_pMatricesCbBegin, &matrices[0], sizeof(DirectX::XMMATRIX) * m_lightSpaceMat.size());
   
        pCommandlist->SetGraphicsRootDescriptorTable(eCascadeShadowRP::eCascadeShadow_eGsMat, m_matriceCBV);
        return;

    }
    void DirectionalLight12::UpdateLightSpaceMatrices()
    {
        const float nearZ = *m_pCam->GetNearZ();
        const float farZ = *m_pCam->GetFarZ();
        std::vector<float> shadowCacadeLevels = m_pCam->GetCascadeLevels();
        for (UINT i = 0; i < shadowCacadeLevels.size(); ++i)
        {
            if (i == 0)
            {
                m_lightSpaceMat[i] = UpdateLightSpaceMat(nearZ, shadowCacadeLevels[i]);
            }
            else if (i < shadowCacadeLevels.size() - 1)
            {
                m_lightSpaceMat[i] = UpdateLightSpaceMat(shadowCacadeLevels[i - 1], shadowCacadeLevels[i]);
            }
            else
            {
                m_lightSpaceMat[i] = UpdateLightSpaceMat(shadowCacadeLevels[i - 1], farZ);
            }
        }
    }

    std::vector<DirectX::XMVECTOR> DirectionalLight12::GetFrustumCornersWorldSpace(DirectX::XMMATRIX viewMat, DirectX::XMMATRIX projMat)
    {
        DirectX::XMMATRIX viewProMat = DirectX::XMMatrixMultiply(viewMat, projMat);
        DirectX::XMMATRIX invVPMat = DirectX::XMMatrixInverse(nullptr, viewProMat);
        std::vector<DirectX::XMVECTOR> corners;
        corners.reserve(8);
        for (UINT x = 0; x < 2; ++x)
        {
            for (UINT y = 0; y < 2; ++y)
            {
                for (UINT z = 0; z < 2; ++z)
                {
                    DirectX::XMVECTOR corner = DirectX::XMVectorSet(2.0f * x - 1.0f, 2.0f * y - 1.0f, z, 1.0f);
                    corner = DirectX::XMVector4Transform(corner, invVPMat);
                    corner = DirectX::XMVectorScale(corner, 1 / corner.m128_f32[3]);
                    corners.push_back(corner);
                }
            }
        }
        return corners;
    }

    DirectX::XMMATRIX DirectionalLight12::UpdateLightSpaceMat(const float nearZ, const float farZ)
    {
        const float fovY = *(m_pCam->GetFovY());
        const float ratio = m_pCam->GetAspect();

        //Gen subFrustum
        DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(fovY, ratio, nearZ, farZ);
        DirectX::XMMATRIX view = *(m_pCam->GetViewMatrix());
        view = DirectX::XMMatrixTranspose(view);

        const std::vector<DirectX::XMVECTOR> corners = GetFrustumCornersWorldSpace(view, proj);

        DirectX::XMVECTOR center = DirectX::XMVectorZero();
        for (UINT i = 0; i < corners.size(); ++i)
        {
            center = DirectX::XMVectorAdd(center, corners[i]);
        }
        center = DirectX::XMVectorScale(center, (float)1 / corners.size());
        DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&m_direction);
        pos = DirectX::XMVector4Normalize(pos);
        pos = DirectX::XMVectorSubtract(center, pos);

        const DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(pos, center, up);

        float minX = FLT_MAX;
        float maxX = FLT_MIN;
        float minY = FLT_MAX;
        float maxY = FLT_MIN;
        float minZ = FLT_MAX;
        float maxZ = FLT_MIN;

        for (UINT i = 0; i < corners.size(); ++i)
        {
            const DirectX::XMVECTOR vec = DirectX::XMVector4Transform(corners[i], lightView);
            minX = min(minX, vec.m128_f32[0]);
            maxX = max(maxX, vec.m128_f32[0]);
            minY = min(minY, vec.m128_f32[1]);
            maxY = max(maxY, vec.m128_f32[1]);
            minZ = min(minZ, vec.m128_f32[2]);
            maxZ = max(maxZ, vec.m128_f32[2]);

        }

        const DirectX::XMMATRIX lightProj =
            DirectX::XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);

        return DirectX::XMMatrixMultiplyTranspose(lightView, lightProj);
    }


}