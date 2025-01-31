#include "SamplerManager.h"
#include "D3D12.h"
namespace wilson
{
	shared_ptr<SamplerManager> g_pSamplerManager = nullptr;
	void SamplerManager::UploadWrapSampler(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT idx, bool bCompute)
	{
		if (bCompute)
		{
			pCmdList->SetComputeRootDescriptorTable(idx, m_wrapSsv);
		}
		else
		{
			pCmdList->SetGraphicsRootDescriptorTable(idx, m_wrapSsv);
		}
	}
	void SamplerManager::UploadClampLinearSampler(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT idx, bool bCompute)
	{
		if (bCompute)
		{
			pCmdList->SetComputeRootDescriptorTable(idx, m_clampLinear);
		}
		else
		{
			pCmdList->SetGraphicsRootDescriptorTable(idx, m_clampLinear);
		}
	}
	void SamplerManager::UploadClampPointSampler(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT idx, bool bCompute)
	{
		if (bCompute)
		{
			pCmdList->SetComputeRootDescriptorTable(idx, m_clampPoint);
		}
		else
		{
			pCmdList->SetGraphicsRootDescriptorTable(idx, m_clampPoint);
		}
	}
	void SamplerManager::UploadBorderSampler(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT idx, bool bCompute)
	{
		if (bCompute)
		{
			pCmdList->SetComputeRootDescriptorTable(idx, m_borderSsv);
		}
		else
		{
			pCmdList->SetGraphicsRootDescriptorTable(idx, m_borderSsv);
		}
	}
	void SamplerManager::UploadBorderLinearPoint(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT idx, bool bCompute)
	{
		if (bCompute)
		{
			pCmdList->SetComputeRootDescriptorTable(idx, m_borderLinearPoint);
		}
		else
		{
			pCmdList->SetGraphicsRootDescriptorTable(idx, m_borderLinearPoint);
		}
	}
	void SamplerManager::UploadBorderLinearPointLess(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT idx, bool bCompute)
	{
		if (bCompute)
		{
			pCmdList->SetComputeRootDescriptorTable(idx, m_borderLinearPointLess);
		}
		else
		{
			pCmdList->SetGraphicsRootDescriptorTable(idx, m_borderLinearPointLess);
		}
	}
	SamplerManager::SamplerManager()
	{
		ComPtr<ID3D12Device> pDevice = D3D12::GetDevice();
		{
			D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = g_pHeapManager->GetCurSamplerCpuHandle();
			D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = g_pHeapManager->GetCurSamplerGpuHandle();

			D3D12_SAMPLER_DESC wrapSamplerDesc = {};
			wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			wrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			wrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			wrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			pDevice->CreateSampler(&wrapSamplerDesc, samplerCpuHandle);

			m_wrapSsv = samplerGpuHandle;
			g_pHeapManager->IncreaseSamplerHandleOffset();
		}


		{
			D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = g_pHeapManager->GetCurSamplerCpuHandle();
			D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = g_pHeapManager->GetCurSamplerGpuHandle();

			D3D12_SAMPLER_DESC clampSamplerDesc = {};
			clampSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			clampSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			clampSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			clampSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			pDevice->CreateSampler(&clampSamplerDesc, samplerCpuHandle);

			m_clampLinear = samplerGpuHandle;
			g_pHeapManager->IncreaseSamplerHandleOffset();
		}

		{
			D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = g_pHeapManager->GetCurSamplerCpuHandle();
			D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = g_pHeapManager->GetCurSamplerGpuHandle();

			D3D12_SAMPLER_DESC clampSamplerDesc = {};
			clampSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			clampSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			clampSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			clampSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			pDevice->CreateSampler(&clampSamplerDesc, samplerCpuHandle);

			m_clampPoint = samplerGpuHandle;
			g_pHeapManager->IncreaseSamplerHandleOffset();
		}

		{
			D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = g_pHeapManager->GetCurSamplerCpuHandle();
			D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = g_pHeapManager->GetCurSamplerGpuHandle();

			D3D12_SAMPLER_DESC borderSamplerDesc = {};
			borderSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			borderSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			borderSamplerDesc.AddressV = borderSamplerDesc.AddressU;
			borderSamplerDesc.AddressW = borderSamplerDesc.AddressU;
			borderSamplerDesc.MaxLOD = 16.0f;
			pDevice->CreateSampler(&borderSamplerDesc, samplerCpuHandle);

			m_borderSsv = samplerGpuHandle;
			g_pHeapManager->IncreaseSamplerHandleOffset();
		}

		//Gen Samplers
		{

			D3D12_SAMPLER_DESC ssDesc = {};
			ssDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			ssDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			ssDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			ssDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			ssDesc.BorderColor[0] = 1.0f;
			ssDesc.BorderColor[1] = 1.0f;
			ssDesc.BorderColor[2] = 1.0f;
			ssDesc.BorderColor[3] = 1.0f;
			ssDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;

			{
				

				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = g_pHeapManager->GetCurSamplerCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = g_pHeapManager->GetCurSamplerGpuHandle();

				pDevice->CreateSampler(&ssDesc, samplerCpuHandle);
				m_borderLinearPoint = samplerGpuHandle;
				g_pHeapManager->IncreaseSamplerHandleOffset();
			}

			{
				ssDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;

				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = g_pHeapManager->GetCurSamplerCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = g_pHeapManager->GetCurSamplerGpuHandle();

				pDevice->CreateSampler(&ssDesc, samplerCpuHandle);
				m_borderLinearPointLess = samplerGpuHandle;
				g_pHeapManager->IncreaseSamplerHandleOffset();

			}


		}
	}
}

