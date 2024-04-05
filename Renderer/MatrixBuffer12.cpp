#include "MatrixBuffer12.h"
#include "HeapManager.h"
namespace wilson
{
	MatBuffer12::MatBuffer12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandlist, HeapManager* pHeapManager,
		XMMATRIX* pViewMat, XMMATRIX* pProjMat)
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
		m_matCBV = pHeapManager->GetCBV(cbSize, pDevice);
		
		cbSize = sizeof(DirectX::XMMATRIX);
		m_pProjMatCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
		m_projMatCBV = pHeapManager->GetCBV(cbSize, pDevice);
		
		cbSize = sizeof(DirectX::XMMATRIX);
		m_pViewMatCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
		m_viewMatCBV = pHeapManager->GetCBV(cbSize, pDevice);

		cbSize = sizeof(DirectX::XMMATRIX);
		m_pCombinedMatCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
		m_combinedMatCBV = pHeapManager->GetCBV(cbSize, pDevice);
	
	}

	MatBuffer12::~MatBuffer12()
	{
	}

	void MatBuffer12::UploadMatBuffer(ID3D12GraphicsCommandList* pCommandlist)
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
		pCommandlist->SetGraphicsRootDescriptorTable(0, m_matCBV);
		return;
	}

	void MatBuffer12::UploadProjMat(ID3D12GraphicsCommandList* pCommandlist, bool bSSAO)
	{	
		memcpy(m_pProjMatCbBegin, &m_projMat, sizeof(XMMATRIX));
		if (bSSAO)
		{
			pCommandlist->SetComputeRootDescriptorTable(eSsao_eCsProj,  m_projMatCBV);
		}
		else
		{
			pCommandlist->SetGraphicsRootDescriptorTable( ePbrLight_ePsProjMat, m_projMatCBV);
		}
		return;
	}
	void MatBuffer12::UploadViewMat(ID3D12GraphicsCommandList* pCommandlist)
	{
		memcpy(m_pViewMatCbBegin, &m_viewMat, sizeof(XMMATRIX));
		pCommandlist->SetGraphicsRootDescriptorTable(ePbrLight_ePsViewMat, m_viewMatCBV);
		
	}
	void MatBuffer12::UploadCombinedMat(ID3D12GraphicsCommandList* pCommandlist, bool bSpotShadowPass)
	{
		const void* pSrc = bSpotShadowPass ? &m_wvpLitMat : &m_wvpMat;
		memcpy(m_pCombinedMatCbBegin, pSrc, sizeof(XMMATRIX));
		//Zpass, Skybox, SpotShadow, OutlinerTest Pass¿¡ ÀÌ¿ëµÊ 
		pCommandlist->SetGraphicsRootDescriptorTable(0, m_combinedMatCBV);
		return;
	}
	void MatBuffer12::UpdateCombinedMat(bool bSpotShadowPass)
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
