
#include"DirectionalLight11.h"

namespace wilson
{
    DirectionalLight11::DirectionalLight11(ID3D11Device* const pDevice, const UINT idx, Camera11* const pCam)
        :Light11(idx)
    {
        m_pCam = pCam;
        m_lightSpaceMat.resize(pCam->GetCascadeLevels().size());
        UpdateLightSpaceMatrices();

        m_pMatriceCb = nullptr;

        {
            D3D11_BUFFER_DESC cbufferDesc = {};
            cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            cbufferDesc.ByteWidth = sizeof(DirLightProperty);
            cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            cbufferDesc.MiscFlags = 0;
            cbufferDesc.StructureByteStride = 0;

            HRESULT hr = pDevice->CreateBuffer(&cbufferDesc, nullptr, &m_pLightCb);
            assert(SUCCEEDED(hr));
            m_pLightCb->SetPrivateData(WKPDID_D3DDebugObjectName,
                sizeof("DirectionalLight11::m_pLightCb") - 1, "DirectionalLight11::m_pLightCb");



            cbufferDesc.ByteWidth = sizeof(DirectX::XMMATRIX) * m_pCam->GetCascadeLevels().size();
            hr = pDevice->CreateBuffer(&cbufferDesc, nullptr, &m_pMatriceCb);
            assert(SUCCEEDED(hr));
            m_pMatriceCb->SetPrivateData(WKPDID_D3DDebugObjectName,
                sizeof("DirectionalLight11::m_pMatriceCb") - 1, "DirectionalLight11::m_pMatriceCb");

        }
    }

    DirectionalLight11::~DirectionalLight11()
    {   
        if (m_pMatriceCb != nullptr)
        {
            m_pMatriceCb->Release();
            m_pMatriceCb = nullptr;
        }
        Light11::~Light11();
    }
   
    void DirectionalLight11::UpdateProperty()
    {
        m_dirLightProperty.ambient = m_ambient;
        m_dirLightProperty.diffuse = m_diffuse;
        m_dirLightProperty.specular = m_specular;
        m_dirLightProperty.direction = m_direction;
    }

    void DirectionalLight11::UploadShadowMatrices(ID3D11DeviceContext* const  pContext)
    {   
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DirectX::XMMATRIX* pMatrix;
        HRESULT hr;
        hr = pContext->Map(m_pMatriceCb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        assert(SUCCEEDED(hr));
        pMatrix = reinterpret_cast<DirectX::XMMATRIX*>(mappedResource.pData);
        for (int i = 0; i < m_lightSpaceMat.size(); ++i)
        {
            pMatrix[i] = m_lightSpaceMat[i];
        }
        pContext->Unmap(m_pMatriceCb, 0);
        pContext->GSSetConstantBuffers(0, 1, &m_pMatriceCb);
        return;

    }
    void DirectionalLight11::UpdateLightSpaceMatrices()
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

    std::vector<DirectX::XMVECTOR> DirectionalLight11::GetFrustumCornersWorldSpace(const DirectX::XMMATRIX viewMat, const DirectX::XMMATRIX projMat)
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

    DirectX::XMMATRIX DirectionalLight11::UpdateLightSpaceMat(const float nearZ, const float farZ)
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