#include "Camera12.h"
#include "DescriptorHeapManager.h"
#include "typedef.h"
namespace wilson {

	Camera12::Camera12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandlist, DescriptorHeapManager* pDescriptorHeapManager,
		const UINT screenWidth, const UINT screenHeight, float screenFar, float screenNear)
	{
		m_fScreenNear = screenNear;
		m_fScreenFar = screenFar;
		m_fFOV = static_cast<float>(3.1459) / 4.0f;
		m_fScreenRatio = screenWidth / static_cast<float>(screenHeight);
		m_trSpeed = 0.1f;
		m_rtSpeed = 0.0175f;
		UpdateCascadeLevels();


		ResetTranslation();
		ResetRotation();
		m_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		m_worldUp = m_up;

		m_projMat = DirectX::XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_fScreenNear, m_fScreenFar);
		m_viewMat = DirectX::XMMatrixLookAtLH(m_pos, m_target, m_up);
		m_viewMat = DirectX::XMMatrixTranspose(m_viewMat);
		m_projMat = DirectX::XMMatrixTranspose(m_projMat);

		m_pCamCb = nullptr;
		m_pCamPosCbBegin = nullptr;
		m_pCascadeLevelCbBegin = nullptr;

		HRESULT hr;


		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC cbufferDesc = {};
		cbufferDesc.Width = _64KB_ALIGN(sizeof(CamBuffer));
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
				&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pCamCb));
			if (FAILED(hr))
			{
				OutputDebugStringA("Camera::m_pCamCb::CreateBufferFailed");
			}
			m_pCamCb->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Camera::m_pCamCb") - 1, "Camera::m_pCamCb");

			D3D12_RANGE readRange = { 0, };
			hr = m_pCamCb->Map(0, &readRange, reinterpret_cast<void**>(&m_pCamPosCbBegin));
			if (FAILED(hr))
			{
				OutputDebugStringA("Camera::m_pCamCb::Map()Failed");
			}

			UINT constantBufferSize = sizeof(CamBuffer);
			m_pCascadeLevelCbBegin = m_pCamPosCbBegin + (_CBV_ALIGN(constantBufferSize));
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);//255aligned
			cbvDesc.BufferLocation = m_pCamCb->GetGPUVirtualAddress();
			pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
			m_camPosCBV = cbvSrvGpuHandle;
			pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

		}


		{
			D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
			D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

			UINT constantBufferSize = sizeof(DirectX::XMVECTOR) * _CASCADE_LEVELS;
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
			cbvDesc.BufferLocation = m_pCamCb->GetGPUVirtualAddress()+256;
			pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
			m_cascadeLevelCBV = cbvSrvGpuHandle;
			pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
		}
		

	}

	Camera12::~Camera12()
	{

		if (m_pCamCb != nullptr)
		{
			m_pCamCb->Release();
			m_pCamCb = nullptr;
		}

	}

	void Camera12::ResetTranslation()
	{
		m_pos = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
		m_target = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	}

	void Camera12::UpdateCascadeLevels()
	{
		m_shadowCascadeLevels = std::vector({ m_fScreenFar / 100.0f,  m_fScreenFar / 50.0f,  m_fScreenFar / 25.0f,  m_fScreenFar / 5.0f , m_fScreenFar });
	}


	void Camera12::Rotate(int dpitch, int dyaw)
	{
		DirectX::XMFLOAT4 float4;
		DirectX::XMStoreFloat4(&float4, m_rotation);
		float pitch = float4.x + dpitch * m_rtSpeed;
		float yaw = float4.y + dyaw * m_rtSpeed;

		if (pitch < 0.0f)
		{
			pitch += _RAD;
		}
		if (yaw < 0.0f)
		{
			yaw += _RAD;
		}
		pitch = pitch > _RAD ? 0 : pitch;
		yaw = yaw > _RAD ? 0 : yaw;

		m_rotation = DirectX::XMVectorSet(pitch, yaw, 0.0f, 0.0f);
	}

	void Camera12::Translate(DirectX::XMVECTOR dv)
	{
		DirectX::XMMATRIX rtMat = DirectX::XMMatrixRotationRollPitchYawFromVector(m_rotation);
		dv = DirectX::XMVector3Transform(dv, rtMat);
		dv = DirectX::XMVectorScale(dv, m_trSpeed);

		m_pos = DirectX::XMVectorAdd(m_pos, dv);
	}

	void Camera12::Update()
	{
		DirectX::XMMATRIX rtMat = DirectX::XMMatrixRotationRollPitchYawFromVector(m_rotation);
		m_dir = DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), rtMat);
		m_dir = DirectX::XMVector3Normalize(m_dir);
		m_target = DirectX::XMVectorAdd(m_dir, m_pos);


		m_right = DirectX::XMVector3Cross(m_dir, m_worldUp);
		m_right = DirectX::XMVector3Normalize(m_right);

		m_up = DirectX::XMVector3Cross(m_right, m_dir);
		m_up = DirectX::XMVector3Normalize(m_up);

		m_viewMat = DirectX::XMMatrixLookAtLH(m_pos, m_target, m_up);
		m_viewMat = DirectX::XMMatrixTranspose(m_viewMat);
		m_projMat = DirectX::XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_fScreenNear, m_fScreenFar);
		m_projMat = DirectX::XMMatrixTranspose(m_projMat);
		UpdateCascadeLevels();
	}

	bool Camera12::SetCascadeLevels(ID3D12GraphicsCommandList* pCommandlist)
	{
		std::vector<DirectX::XMVECTOR> FarZs(_CASCADE_LEVELS);
		for (int i = 0; i < _CASCADE_LEVELS; ++i)
		{
			FarZs[i] = DirectX::XMVectorSet(0, 0, m_shadowCascadeLevels[i], 1.0f);
		}

		memcpy(m_pCascadeLevelCbBegin, &FarZs[0], sizeof(DirectX::XMVECTOR) * _CASCADE_LEVELS);
		pCommandlist->SetGraphicsRootDescriptorTable(ePbrLightRP::ePbrLight_ePsCasCadeLevels, m_cascadeLevelCBV);
		return true;
	}


	bool Camera12::SetCamPos(ID3D12GraphicsCommandList* pCommandlist, bool bGeoPass)
	{
		memcpy(m_pCamPosCbBegin, &m_pos, sizeof(CamBuffer));
		pCommandlist->SetGraphicsRootDescriptorTable(bGeoPass?ePbrGeoRP::ePbrGeo_ePsCamPos: ePbrLightRP::ePbrLight_ePsCamPos,
			m_camPosCBV);
		return true;

	}

}