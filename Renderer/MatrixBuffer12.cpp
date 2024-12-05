#include "MatrixBuffer12.h"
#include "HeapManager.h"
namespace wilson
{
	MatrixHandler12::MatrixHandler12(ID3D12Device* const pDevice, ID3D12GraphicsCommandList* const pCommandlist, HeapManager* const pHeapManager,
		XMMATRIX* const pViewMat, XMMATRIX* const pProjMat)
	{

		m_pMatricesCbBegin = nullptr;
		m_pProjMatCbBegin = nullptr;
		m_pViewMatCbBegin = nullptr;
		m_pCombinedMatCbBegin = nullptr;

		bDirty = false;

		m_worldMat = XMMatrixIdentity();
		m_matrixBuffer.worldMat = m_worldMat;
		m_invWorldMat = m_worldMat;
		m_matrixBuffer.invWorldMat= m_invWorldMat;
		m_lightSpaceMat = m_worldMat;
		m_extraMat = m_worldMat;

		m_viewMat = *pViewMat;
		m_matrixBuffer.viewMat = m_viewMat;
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

	MatrixHandler12::~MatrixHandler12()
	{
	}

	void MatrixHandler12::UploadMatBuffer(ID3D12GraphicsCommandList* const pCommandlist)
	{	
		pCommandlist->SetGraphicsRootDescriptorTable(0, m_matCbv);
		return;
	}
	void MatrixHandler12::SetProjMatrix(XMMATRIX* const projMat)
	{
		m_projMat = *projMat;
		memcpy(m_pProjMatCbBegin, &m_projMat, sizeof(XMMATRIX));
	}
	void MatrixHandler12::UploadProjMat(ID3D12GraphicsCommandList* const pCommandlist, const bool bSsao)
	{	
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
	void MatrixHandler12::SetViewMatrix(XMMATRIX* const viewMatrix)
	{
		m_viewMat = *viewMatrix;
		m_matrixBuffer.viewMat = m_viewMat;
		memcpy(m_pViewMatCbBegin, &m_viewMat, sizeof(XMMATRIX));
	}
	void MatrixHandler12::UploadViewMat(ID3D12GraphicsCommandList* const pCommandlist)
	{
		pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psViewMat), m_viewMatCbv);
		
	}
	void MatrixHandler12::SetWorldMatrix(XMMATRIX* const worldMatrix)
	{
		m_worldMat = *worldMatrix;
		m_matrixBuffer.worldMat = m_worldMat;
	}
	void MatrixHandler12::SetInvWorldMatrix(XMMATRIX* const invWorldMatrix)
	{
		m_invWorldMat = *invWorldMatrix;
		m_matrixBuffer.invWorldMat = m_invWorldMat;
	}
	void MatrixHandler12::UploadCombinedMat(ID3D12GraphicsCommandList* const pCommandlist)
	{
		//Zpass, Skybox, SpotShadow, Pass¿¡ ÀÌ¿ëµÊ 
		pCommandlist->SetGraphicsRootDescriptorTable(0, m_combinedMatCbv);
		return;
	}
	void MatrixHandler12::UpdateCombinedMat(const bool bSpotShadowPass)
	{
		if (bSpotShadowPass)
		{
			m_wvpLitMat = XMMatrixMultiplyTranspose(XMMatrixTranspose(m_worldMat), XMMatrixTranspose(m_lightSpaceMat));
			memcpy(m_pCombinedMatCbBegin + sizeof(XMMATRIX), &m_wvpLitMat, sizeof(XMMATRIX));
		}
		else
		{
			XMMATRIX wv = XMMatrixMultiply(XMMatrixTranspose(m_worldMat), XMMatrixTranspose(m_viewMat));
			m_wvpMat = XMMatrixMultiply(wv, XMMatrixTranspose(m_projMat));
			m_wvpTransposedMat = XMMatrixTranspose(m_wvpMat);
			memcpy(m_pCombinedMatCbBegin, &m_wvpTransposedMat, sizeof(XMMATRIX));
			m_matrixBuffer.wvpTransposedMat = m_wvpTransposedMat;
		}
		
		//비트로 어떤 행렬이 갱신되었는지 추적
		{
			XMMATRIX invWVMat = XMMatrixMultiply(XMMatrixTranspose(m_worldMat), XMMatrixTranspose(m_viewMat));
			invWVMat = XMMatrixInverse(nullptr, invWVMat);
			m_invWVMat = invWVMat;
			memcpy(m_pMatricesCbBegin, &m_matrixBuffer, sizeof(MatrixBuffer));
		}

	}

}
