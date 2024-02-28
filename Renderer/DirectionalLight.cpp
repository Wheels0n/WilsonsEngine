#include<cmath>

#include"DirectionalLight.h"

namespace wilson
{
    DirectionalLight::DirectionalLight(ID3D11Device* pDevice, UINT idx, Camera* pCam)
        :Light(idx)
    {
        m_pCam = pCam;
        m_lightSpaceMat.resize(pCam->GetCascadeLevels().size());
        UpdateLightSpaceMatrices();

        m_pMatriceBuffer = nullptr;

        {
            D3D11_BUFFER_DESC cbufferDesc = {};
            cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            cbufferDesc.ByteWidth = sizeof(DirLightProperty);
            cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            cbufferDesc.MiscFlags = 0;
            cbufferDesc.StructureByteStride = 0;

            HRESULT hr = pDevice->CreateBuffer(&cbufferDesc, nullptr, &m_pLightBuffer);
            assert(SUCCEEDED(hr));
            m_pLightBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
                sizeof("DirectionalLight::m_pLightBuffer") - 1, "DirectionalLight::m_pLightBuffer");



            cbufferDesc.ByteWidth = sizeof(DirectX::XMMATRIX) * m_pCam->GetCascadeLevels().size();
            hr = pDevice->CreateBuffer(&cbufferDesc, nullptr, &m_pMatriceBuffer);
            assert(SUCCEEDED(hr));
            m_pMatriceBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
                sizeof("DirectionalLight::m_pMatriceBuffer") - 1, "DirectionalLight::m_pMatriceBuffer");

        }
    }

    DirectionalLight::~DirectionalLight()
    {   
        if (m_pMatriceBuffer != nullptr)
        {
            m_pMatriceBuffer->Release();
            m_pMatriceBuffer = nullptr;
        }
        Light::~Light();
    }
   
    void DirectionalLight::UpdateProperty()
    {
        m_dirLightProperty.ambient = m_ambient;
        m_dirLightProperty.diffuse = m_diffuse;
        m_dirLightProperty.specular = m_specular;
        m_dirLightProperty.direction = m_direction;
    }

    void DirectionalLight::UploadShadowMatrices(ID3D11DeviceContext* pContext)
    {   
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DirectX::XMMATRIX* pMatrix;
        HRESULT hr;
        hr = pContext->Map(m_pMatriceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        assert(SUCCEEDED(hr));
        pMatrix = (DirectX::XMMATRIX*)mappedResource.pData;
        for (int i = 0; i < m_lightSpaceMat.size(); ++i)
        {
            pMatrix[i] = m_lightSpaceMat[i];
        }
        pContext->Unmap(m_pMatriceBuffer, 0);
        pContext->GSSetConstantBuffers(0, 1, &m_pMatriceBuffer);
        return;

    }
    void DirectionalLight::UpdateLightSpaceMatrices()
    {   
        const float nearZ = *m_pCam->GetNearZ();
        const float farZ = *m_pCam->GetFarZ();
        std::vector<float> shadowCacadeLevels=m_pCam->GetCascadeLevels();
        for (UINT i = 0; i < shadowCacadeLevels.size(); ++i)
        {
            if (i == 0)
            {
                m_lightSpaceMat[i] = UpdateLightSpaceMat(nearZ, shadowCacadeLevels[i]);
            }
            else if (i < shadowCacadeLevels.size()-1)
            {
                m_lightSpaceMat[i] = UpdateLightSpaceMat(shadowCacadeLevels[i - 1], shadowCacadeLevels[i]);
            }
            else
            {
                m_lightSpaceMat[i]= UpdateLightSpaceMat(shadowCacadeLevels[i-1],farZ);
            }
        }
    }

    std::vector<DirectX::XMVECTOR> DirectionalLight::GetFrustumCornersWorldSpace(DirectX::XMMATRIX viewMat, DirectX::XMMATRIX projMat)
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

    DirectX::XMMATRIX DirectionalLight::UpdateLightSpaceMat(const float nearZ, const float farZ)
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
        center = DirectX::XMVectorScale(center, (float)1/corners.size());
        DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&m_direction);
        pos = DirectX::XMVector4Normalize(pos);
        pos = DirectX::XMVectorSubtract(center,pos);
        
        const DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(pos,center, up);

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