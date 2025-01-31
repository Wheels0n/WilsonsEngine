#pragma once
#include <d3d12.h>
#include "typedef.h"
namespace wilson
{
	using namespace std;
	using namespace Microsoft::WRL;
	class SamplerManager
	{
	public:
		void UploadWrapSampler(ComPtr<ID3D12GraphicsCommandList>, UINT, bool);
		void UploadClampLinearSampler(ComPtr<ID3D12GraphicsCommandList>, UINT, bool);
		void UploadClampPointSampler(ComPtr<ID3D12GraphicsCommandList>, UINT, bool);
		void UploadBorderSampler(ComPtr<ID3D12GraphicsCommandList>, UINT, bool);

		void UploadBorderLinearPoint(ComPtr<ID3D12GraphicsCommandList>, UINT, bool);
		void UploadBorderLinearPointLess(ComPtr<ID3D12GraphicsCommandList>, UINT, bool);
		SamplerManager();
	private:
		D3D12_GPU_DESCRIPTOR_HANDLE m_borderSsv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_clampLinear;
		D3D12_GPU_DESCRIPTOR_HANDLE m_clampPoint;
		D3D12_GPU_DESCRIPTOR_HANDLE m_wrapSsv;

		D3D12_GPU_DESCRIPTOR_HANDLE	m_borderLinearPoint;
		D3D12_GPU_DESCRIPTOR_HANDLE	m_borderLinearPointLess;
	};
	extern shared_ptr<SamplerManager> g_pSamplerManager;
}


