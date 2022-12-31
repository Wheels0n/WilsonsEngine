#include "Model.h"

Cmodel::Cmodel()
{   
    m_uploadBuffer = nullptr;
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
    m_cbvHeap = nullptr;
    m_serializedBlob = nullptr;
    m_errorBlob = nullptr;
}

Cmodel::~Cmodel()
{
}

bool Cmodel::Initialize(ID3D12Device* m_pDevice)
{  

    bool result;

    result = InitializeBuffers(m_pDevice);

    if (result == false)
    {
        return false;
    }

    return true;
}

void Cmodel::Shutdown()
{  
    m_uploadBuffer->Release();
    m_vertexBuffer->Release();
    m_indexBuffer->Release();

}

void Cmodel::Render(ID3D12Device* m_pDevice, ID3D12GraphicsCommandList* pCmdList)
{   
    D3D12_RESOURCE_BARRIER barrier[2];
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    D3D12_DESCRIPTOR_RANGE cbvTable;
    D3D12_GPU_DESCRIPTOR_HANDLE cbv = { 0 };
    D3D12_ROOT_PARAMETER slotRootParameter[1];
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc;
    ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap };
    D3D12_RASTERIZER_DESC rasterizerDesc;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso;
    unsigned char* pMappedData = nullptr;
    HRESULT hr;

    barrier[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier[0].Transition.pResource = m_vertexBuffer;
    barrier[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    barrier[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  

    pCmdList->ResourceBarrier(1, barrier);
    //copy vertex data from cpu to gpu
    pCmdList->CopyBufferRegion(m_vertexBuffer, 0, m_uploadBuffer, 0, sizeof(Vertex) * 3);
    m_constantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pMappedData));
    memcpy(pMappedData, &m_worldViewProj, sizeof(XMFLOAT4X4));
    


    barrier[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier[0].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
   
    pCmdList->ResourceBarrier(1, barrier);
    m_constantBuffer->Unmap(0, nullptr);
    
    pMappedData = nullptr;

    //create vertex buffer view
    D3D12_VERTEX_BUFFER_VIEW vBV;
    vBV.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    vBV.SizeInBytes = sizeof(Vertex) * 3;
    vBV.StrideInBytes = sizeof(Vertex);

    //create constant buffer view and its table for root signature 
    cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = max(256,sizeof(XMFLOAT4X4)); // 버퍼 초기화 당시 크기만큼 가상메모리 공간이 할당되니 주의
    m_pDevice->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
    
    cbvTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    cbvTable.NumDescriptors = 1;
    cbvTable.RegisterSpace = 0;
    cbvTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    
    //set root param to the root table 
    slotRootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    slotRootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    slotRootParameter[0].DescriptorTable.NumDescriptorRanges = 1;
    slotRootParameter[0].DescriptorTable.pDescriptorRanges = &cbvTable;

  
    rootSigDesc.NumParameters = 1;
    rootSigDesc.pParameters = slotRootParameter;
    rootSigDesc.NumStaticSamplers = 0;
    rootSigDesc.pStaticSamplers = nullptr;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &m_serializedBlob, &m_errorBlob); 
    m_pDevice->CreateRootSignature(0, m_serializedBlob->GetBufferPointer(), m_serializedBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(&m_RootSignature));

    pCmdList->SetGraphicsRootSignature(m_RootSignature);
    pCmdList->SetDescriptorHeaps(1, descriptorHeaps);

    //set cbv handle 
    cbv = m_cbvHeap->GetGPUDescriptorHandleForHeapStart();
    cbv.ptr = static_cast<unsigned long long> (static_cast<signed long long>(cbv.ptr) + static_cast<signed long long>(0) * static_cast<signed long long>(cbvDesc.SizeInBytes));
    pCmdList->SetGraphicsRootDescriptorTable(0, cbv);

    //get Binary shader file
    hr = D3DCompileFromFile(L"VS.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &m_vsBlob, &m_errorBlob);
    if (FAILED(hr))
    {
        return;
    }
    hr = D3DCompileFromFile(L"PS.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &m_psBlob, &m_errorBlob);
    if (FAILED(hr))
    {
        return;
    }

    //rasterizerDesc
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable = true;
    rasterizerDesc.MultisampleEnable = false;
    rasterizerDesc.AntialiasedLineEnable = false;
    rasterizerDesc.ForcedSampleCount = 0;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    //fill pso
    ZeroMemory(&pso, sizeof(pso));
    pso.pRootSignature = m_RootSignature;
    pso.VS.pShaderBytecode = m_vsBlob->GetBufferPointer();
    pso.VS.BytecodeLength = m_vsBlob->GetBufferSize();
    pso.PS.pShaderBytecode = m_psBlob->GetBufferPointer();
    pso.PS.BytecodeLength = m_psBlob->GetBufferSize();
    pso.RasterizerState = rasterizerDesc;

    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
    {
        FALSE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };
    pso.BlendState.AlphaToCoverageEnable = false;
    pso.BlendState.IndependentBlendEnable = true;
    pso.BlendState.RenderTarget[0] = defaultRenderTargetBlendDesc;

    pso.DepthStencilState.DepthEnable = true;
    pso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    pso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    pso.DepthStencilState.StencilEnable = true;
    pso.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    pso.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
    { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
    pso.DepthStencilState.FrontFace = defaultStencilOp;
    pso.DepthStencilState.BackFace = defaultStencilOp;

    pso.SampleMask = UINT_MAX;
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso.SampleDesc.Count = 1;
    pso.SampleDesc.Quality = 0;
    pso.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;;

    
    //Draw
    pCmdList->IASetVertexBuffers(0, 1, &vBV);
    pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
    pCmdList->DrawInstanced(3, 1, 0, 0);
}

bool Cmodel::InitializeBuffers(ID3D12Device* m_pDevice)
{
    HRESULT hr;
    D3D12_INPUT_ELEMENT_DESC vertexDesc[2];
    D3D12_INPUT_LAYOUT_DESC  InputLayoutDesc;
    D3D12_RESOURCE_DESC vertexBufferDesc, constantBufferDesc;;
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    D3D12_HEAP_PROPERTIES uploadHeapProps, defaultHeapProps;
    D3D12_SUBRESOURCE_DATA subResourceData = { 0, };

    Vertex* verticeCoordinates;
    unsigned long* indices;


    m_vertexCnt = 3;
    m_indexCnt = 3;

    verticeCoordinates = new Vertex[m_vertexCnt];
    if (verticeCoordinates == nullptr)
    {
        return false;
    }

    indices = new unsigned long[m_indexCnt];
    if (indices == nullptr)
    {
        return false;
    }


    verticeCoordinates[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
    verticeCoordinates[0].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

    verticeCoordinates[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);
    verticeCoordinates[1].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

    verticeCoordinates[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);
    verticeCoordinates[2].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    vertexDesc[0].SemanticName = "position";
    vertexDesc[0].SemanticIndex = 0;
    vertexDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    vertexDesc[0].InputSlot = 0;
    vertexDesc[0].AlignedByteOffset = 0;
    vertexDesc[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    vertexDesc[0].InstanceDataStepRate = 0;

    vertexDesc[1].SemanticName = "color";
    vertexDesc[1].SemanticIndex = 0;
    vertexDesc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    vertexDesc[1].InputSlot = 0;
    vertexDesc[1].AlignedByteOffset = 12;
    vertexDesc[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    vertexDesc[1].InstanceDataStepRate = 0;


    InputLayoutDesc.NumElements = 2;
    InputLayoutDesc.pInputElementDescs = vertexDesc;

    vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexBufferDesc.Alignment = 0;
    vertexBufferDesc.Width = m_vertexCnt*sizeof(Vertex);
    vertexBufferDesc.Height = 1;
    vertexBufferDesc.DepthOrArraySize = 1;
    vertexBufferDesc.MipLevels = 1;
    vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    vertexBufferDesc.SampleDesc = { 1,0 }; //sampleCnt, sampleQuality
    vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;


    defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    defaultHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    defaultHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    defaultHeapProps.CreationNodeMask = 1;
    defaultHeapProps.VisibleNodeMask = 1;

    m_pDevice->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**> (&m_vertexBuffer));


    uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    uploadHeapProps.CreationNodeMask = 1;
    uploadHeapProps.VisibleNodeMask = 1;

    m_pDevice->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_uploadBuffer));

    subResourceData.pData = verticeCoordinates;
  

    constantBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    constantBufferDesc.Alignment = 0;
    constantBufferDesc.Width = 256;
    constantBufferDesc.Height = 1;
    constantBufferDesc.DepthOrArraySize = 1;
    constantBufferDesc.MipLevels = 1;
    constantBufferDesc.SampleDesc = { 1,0 };
    constantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    constantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    constantBufferDesc.Format = DXGI_FORMAT_UNKNOWN;

    m_pDevice->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**> (&m_constantBuffer));

    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;

    m_pDevice->CreateDescriptorHeap(&cbvHeapDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&m_cbvHeap));
}