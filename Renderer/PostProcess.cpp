#include "PostProcess.h"
#include "HeapManager.h"
namespace wilson
{
	void PostProcess::UpdateExposureParemeters()
	{
		g_pHeapManager->CopyDataToCb(m_exposureCbKey, sizeof(FLOAT), &m_exposure);
	}
	void PostProcess::UploadExposureParemeters(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		g_pHeapManager->UploadComputeCb(m_exposureCbKey, E_TO_UINT(ePostProcessRP::csExposure), pCmdList);
	}
	PostProcess::PostProcess()
		:m_exposure(1.0f)
	{
		m_exposureCbKey = g_pHeapManager->AllocateCb(sizeof(XMFLOAT4));
		UpdateExposureParemeters();
	}
}

