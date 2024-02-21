#include "SpotLight12.h"
#include "DescriptorHeapManager.h"
namespace wilson
{

    SpotLight12::SpotLight12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandlist, DescriptorHeapManager* pDescriptorHeapManager, UINT idx)
        :Light12(idx)
    {
        HRESULT hr;

        D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC cbufferDesc = {};
        cbufferDesc.Width = _64KB_ALIGN(sizeof(SpotLightProperty)); 
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


        hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
            &cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ| D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pLight12Buffer));
        if (FAILED(hr))
        {
            OutputDebugStringA("SpotLight::m_pLight12Buffer::CreateBufferFailed");
        }
        m_pLight12Buffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            sizeof("SpotLight::m_pLight12Buffer") - 1, "SpotLight::m_pLight12Buffer");


        UINT constantBufferSize = cbufferDesc.Width;

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
        cbvDesc.BufferLocation = m_pLight12Buffer->GetGPUVirtualAddress();
        pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
        m_lightCBV = cbvSrvGpuHandle;
        pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
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