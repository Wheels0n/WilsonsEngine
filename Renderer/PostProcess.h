#pragma once
#include <d3d12.h>
#include "typedef.h"
namespace wilson
{
	using namespace Microsoft::WRL;
	class PostProcess
	{
	public : 
		inline FLOAT* GetExposure()
		{
			return &m_exposure;
		}
		void									UpdateExposureParemeters();
		void									UploadExposureParemeters(ComPtr<ID3D12GraphicsCommandList>);

		PostProcess();
	private:
		UINT m_exposureCbKey;
		FLOAT m_exposure;
	};

}

