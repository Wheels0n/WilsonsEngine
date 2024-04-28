#include "MatrixBuffer12.h"
#include "HeapManager.h"
namespace wilson
{
	MatBuffer12::MatBuffer12(ID3D12Device* const pDevice, ID3D12GraphicsCommandList* const pCommandlist, HeapManager* const pHeapManager,
		XMMATRIX* const pViewMat, XMMATRIX* const pProjMat)
	{

		m_pMatricesCbBegin = nullptr;
		m_pProjMatCbBegin = nullptr;
		m_pViewMatCbBegin = nullptr;
		m_pCombinedMatCbBegin = nullptr;

		bDirty = false;

		m_worldMat = XMMatrixIdentity();
		m_invWorldMat = m_worldMat;
		m_lightSpaceMat = m_worldMat;
		m_extraMat = m_worldMat;

		m_viewMat = *pViewMat;
		m_projMat = *pProjMat;

		
		UINT cbSize = sizeof(MatrixBuffer);
		m_pMatricesCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
		m_matCbv = pHeapManager->GetCbv(cbSize, pDevice);
		
		cbSize = sizeof(DirectX::XMMATRIX);
		m_pProjMatCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
		m_projMatCbv = pHeapManager->GetCbv(cbSize, pDevice);
		
		cbSize = sizeof(DirectX::XMMATRIX);
		m_pViewMatCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
		m_viewMatCbv = pHeapManager->GetCbv(cbSize, pDevice);

		cbSize = sizeof(DirectX::XMMATRIX);
		m_pCombinedMatCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
		m_combinedMatCbv = pHeapManager->GetCbv(cbSize, pDevice);
	
	}

	MatBuffer12::~MatBuffer12()
	{
	}

	void MatBuffer12::UploadMatBuffer(ID3D12GraphicsCommandList* const pCommandlist)
	{	
		if (bDirty)
		{
			XMMATRIX invWVMat = XMMatrixMultiply(XMMatrixTranspose(m_worldMat), XMMatrixTranspose(m_viewMat));
			invWVMat = XMMatrixInverse(nullptr, invWVMat);
			invWVMat = XMMatrixTranspose(invWVMat);
			m_invWVMat= invWVMat;
			bDirty = false;
		}
		

		MatrixBuffer matBuffer;
		matBuffer.worldMat = m_worldMat;
		matBuffer.viewMat = m_viewMat;
		matBuffer.invWorldMat = m_invWorldMat;
		matBuffer.invWVMat = m_invWVMat;
		matBuffer.wvpMat = m_wvpMat;


		memcpy(m_pMatricesCbBegin, &matBuffer, sizeof(matBuffer));
		pCommandlist->SetGraphicsRootDescriptorTable(0, m_matCbv);
		return;
	}

	void MatBuffer12::UploadProjMat(ID3D12GraphicsCommandList* const pCommandlist, const bool bSsao)
	{	
		memcpy(m_pProjMatCbBegin, &m_projMat, sizeof(XMMATRIX));
		if (bSsao)
		{
			pCommandlist->SetComputeRootDescriptorTable(static_cast<UINT>(eSsaoRP::csProj),  m_projMatCbv);
		}
		else
		{
			pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psProjMat), m_projMatCbv);
		}
		return;
	}
	void MatBuffer12::UploadViewMat(ID3D12GraphicsCommandList* const pCommandlist)
	{
		memcpy(m_pViewMatCbBegin, &m_viewMat, sizeof(XMMATRIX));
		pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psViewMat), m_viewMatCbv);
		
	}
	void MatBuffer12::UploadCombinedMat(ID3D12GraphicsCommandList* const pCommandlist, const bool bSpotShadowPass)
	{
		const void* pSrc = bSpotShadowPass ? &m_wvpLitMat : &m_wvpMat;
		memcpy(m_pCombinedMatCbBegin, pSrc, sizeof(XMMATRIX));
		//Zpass, Skybox, SpotShadow, OutlinerTest Pass¿¡ ÀÌ¿ëµÊ 
		pCommandlist->SetGraphicsRootDescriptorTable(0, m_combinedMatCbv);
		return;
	}
	void MatBuffer12::UpdateCombinedMat(const bool bSpotShadowPass)
	{
		if (bSpotShadowPass)
		{
			m_wvpLitMat = XMMatrixMultiplyTranspose(XMMatrixTranspose(m_worldMat), XMMatrixTranspose(m_lightSpaceMat));
		}
		else
		{
			XMMATRIX wv = XMMatrixMultiply(XMMatrixTranspose(m_worldMat), XMMatrixTranspose(m_viewMat));
			m_wvpMat = XMMatrixMultiplyTranspose(wv, XMMatrixTranspose(m_projMat));
		}
		
	}
	MatrixBuffer MatBuffer12::GetMatrixBuffer()
	{
		MatrixBuffer matrixBuffer;
		matrixBuffer.worldMat = m_worldMat;
		matrixBuffer.viewMat = m_viewMat;
		matrixBuffer.invWorldMat = m_invWorldMat;
		matrixBuffer.invWVMat = m_invWVMat;
		matrixBuffer.wvpMat = m_wvpMat;

		return matrixBuffer;
	}
}
