#include<cmath>

#include"DirectionalLight12.h"
#include"DescriptorHeapManager.h"
namespace wilson
{
 
    DirectionalLight12::DirectionalLight12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandlist, DescriptorHeapManager* pDescriptorHeapManager, 
        UINT idx, Camera12* pCam)
        :Light12(idx)
    {
        m_pCam = pCam;
        m_lightSpaceMat.resize(pCam->GetCascadeLevels().size());
        UpdateLightSpaceMatrices();

        m_pMatrice12Buffer = nullptr;
        m_pMatricesCbBegin = nullptr;

        {
            HRESULT hr;
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
            heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heapProps.CreationNodeMask = 1;
            heapProps.VisibleNodeMask = 1;

            D3D12_RESOURCE_DESC cbufferDesc = {};
            cbufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
            cbufferDesc.Alignment = 0;
            cbufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            cbufferDesc.Format = DXGI_FORMAT_UNKNOWN;
            cbufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            cbufferDesc.Height = 1;
            cbufferDesc.DepthOrArraySize = 1;
            cbufferDesc.MipLevels = 1;
            cbufferDesc.SampleDesc.Count = 1;
            cbufferDesc.SampleDesc.Quality = 0;
       

            {
                D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
                D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

                cbufferDesc.Width = sizeof(DirectX::XMMATRIX) * _CASCADE_LEVELS;
                cbufferDesc.Width = _64KB_ALIGN(cbufferDesc.Width);

                hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                    &cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pMatrice12Buffer));
                assert(SUCCEEDED(hr));
                m_pMatrice12Buffer->SetPrivateData(WKPDID_D3DDebugObjectName,
                    sizeof("DirectionalLight::m_pMatrices12Buffer") - 1, "DirectionalLight::m_pMatrices12Buffer");

                D3D12_RANGE readRange = { 0, };
                hr = m_pMatrice12Buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pMatricesCbBegin));
                assert(SUCCEEDED(hr));


                UINT constantBufferSize = sizeof(DirectX::XMMATRIX) * _CASCADE_LEVELS;

                D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
                cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
                cbvDesc.BufferLocation = m_pMatrice12Buffer->GetGPUVirtualAddress();
                pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
                m_matriceCBV = cbvSrvGpuHandle;
                pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
            }

        }
    }
    DirectionalLight12::~DirectionalLight12()
    {
        if (m_pMatrice12Buffer != nullptr)
        {
            m_pMatrice12Buffer->Release();
            m_pMatrice12Buffer = nullptr;
        }

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