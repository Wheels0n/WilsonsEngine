#include<DirectXMath.h>
#include "PointLight12.h"
#include "DescriptorHeapManager.h"
namespace wilson
{
    constexpr float g_near = 1.0f;
    constexpr float g_far = 150.0f;
    constexpr float g_ratio = 1.0f;
    constexpr float g_FOV = DirectX::XMConvertToRadians(90.0f);
    DirectX::XMMATRIX PointLight12::g_perspectiveMat =
        DirectX::XMMatrixPerspectiveFovLH(g_FOV, g_ratio, g_near, g_far);
    std::vector<DirectX::XMVECTOR> PointLight12::g_upVectors = {
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    };
    std::vector<DirectX::XMVECTOR> PointLight12::g_dirVectors = {
        DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(-1.0f,0.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f,-1.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
    };


    void PointLight12::UpdateProperty()
    {
        m_pointLightProperty.ambient = m_ambient;
        m_pointLightProperty.attenuation = m_attenuation;
        m_pointLightProperty.diffuse = m_diffuse;
        m_pointLightProperty.position = m_position;
        m_pointLightProperty.range = m_range;
        m_pointLightProperty.specular = m_specular;
    }
    void PointLight12::CreateShadowMatrices()
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
   
    void PointLight12::UploadShadowMatrices(ID3D12GraphicsCommandList* pCommandlist)
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
   
    void PointLight12::UploadLightPos(ID3D12GraphicsCommandList* pCommandlist)
    {
        DirectX::XMVECTOR pos = DirectX::XMVectorSet(m_position.x, m_position.y, m_position.z, g_far);

        memcpy(m_pPosCbBegin, &pos, sizeof(DirectX::XMVECTOR));
        pCommandlist->SetGraphicsRootDescriptorTable(eCubeShadowRP::eCubeShadow_ePsLightPos, m_posCBV);
        return;
    }


    PointLight12::PointLight12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandlist, DescriptorHeapManager* pDescriptorHeapManager, UINT idx)
        :Light12(idx)
    {
        m_pMatrices12Buffer = nullptr;
        m_pPosCb = nullptr;
        m_pMatricesCbBegin = nullptr;
        m_pPosCbBegin = nullptr;
        HRESULT hr;
        {

            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
            heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heapProps.CreationNodeMask = 1;
            heapProps.VisibleNodeMask = 1;

            D3D12_RESOURCE_DESC cbufferDesc = {};
            cbufferDesc.Width = _64KB_ALIGN(sizeof(PointLightProperty));
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

                hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                    &cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pLight12Buffer));
                if (FAILED(hr))
                {
                    OutputDebugStringA("PointlLight::m_pLight12Buffer::CreateBufferFailed");
                }
                m_pLight12Buffer->SetPrivateData(WKPDID_D3DDebugObjectName,
                    sizeof("PointlLight::m_pLight12Buffer") - 1, "PointlLight::m_pLight12Buffer");

                UINT constantBufferSize = sizeof(PointLightProperty);

                D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
                cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
                cbvDesc.BufferLocation = m_pLight12Buffer->GetGPUVirtualAddress();
                pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
                m_lightCBV = cbvSrvGpuHandle;
                pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
            }
          

            {

                D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
                D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

                cbufferDesc.Width = sizeof(DirectX::XMMATRIX) * 7;
                cbufferDesc.Width = _64KB_ALIGN(cbufferDesc.Width);

                hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                    &cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pMatrices12Buffer));
                if (FAILED(hr))
                {
                    OutputDebugStringA("PointlLight::m_pMatrices12Buffer::CreateBufferFailed");
                }
                m_pMatrices12Buffer->SetPrivateData(WKPDID_D3DDebugObjectName,
                    sizeof("PointlLight::m_pMatrices12Buffer") - 1, "PointlLight::m_pMatrices12Buffer");
                
                D3D12_RANGE readRange = { 0, };
                hr = m_pMatrices12Buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pMatricesCbBegin));
                if (FAILED(hr))
                {
                    OutputDebugStringA("PointLight::m_pMatrices12Buffer::Map()Failed");
                }


                UINT constantBufferSize = sizeof(DirectX::XMMATRIX) * 7;

                D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
                cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
                cbvDesc.BufferLocation = m_pMatrices12Buffer->GetGPUVirtualAddress();
                pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
                m_matriceCBV = cbvSrvGpuHandle;
                pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
            }

            {
                D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
                D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

                cbufferDesc.Width = _64KB_ALIGN(sizeof(DirectX::XMVECTOR));

                hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                    &cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pPosCb));
                if (FAILED(hr))
                {
                    OutputDebugStringA("PointlLight::m_pPosCb::CreateBufferFailed");
                }
                m_pPosCb->SetPrivateData(WKPDID_D3DDebugObjectName,
                    sizeof("PointlLight::m_pPosCb") - 1, "PointlLight::m_pPosCb");

                D3D12_RANGE readRange = { 0, };
                hr = m_pPosCb->Map(0, &readRange, reinterpret_cast<void**>(&m_pPosCbBegin));
                if (FAILED(hr))
                {
                    OutputDebugStringA("PointlLight::m_pPosCb::Map()Failed");
                }


                UINT constantBufferSize = cbufferDesc.Width;

                D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
                cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
                cbvDesc.BufferLocation = m_pMatrices12Buffer->GetGPUVirtualAddress();
                pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
                m_posCBV = cbvSrvGpuHandle;
                pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
            }
           
        }
        m_cubeMats.resize(6);
        CreateShadowMatrices();
    }

    PointLight12::~PointLight12()
    {
  
        if (m_pMatrices12Buffer != nullptr)
        {
            m_pMatrices12Buffer->Release();
            m_pMatrices12Buffer = nullptr;
        }

        if (m_pPosCb != nullptr)
        {
            m_pPosCb->Release();
            m_pPosCb = nullptr;
        }
        Light12::~Light12();
    }

}
