#pragma once

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d12.h>

#include "typedef.h"
using namespace DirectX;

namespace wilson
{	
	class HeapManager;
	class MatrixHandler12
	{
	public:

		inline XMMATRIX& GetWVPMatrix()
		{
			return m_wvpMat;
		}
		MatrixBuffer& GetMatrixBuffer()
		{
			return m_matrixBuffer;
		}
		inline bool GetDirtyBit()
		{
			return bDirty;
		}
		inline void SetDirtyBit(bool dirty)
		{
			bDirty = dirty;
		}
		inline void SetLightSpaceMatrix(XMMATRIX* const plightSpaceMat)
		{
			m_lightSpaceMat = *plightSpaceMat;
		}
		void SetWorldMatrix(XMMATRIX* const worldMatrix);
		void SetInvWorldMatrix(XMMATRIX* const invWorldMatrix);
		void SetViewMatrix(XMMATRIX* const viewMatrix);
		void SetProjMatrix(XMMATRIX* const projMat);
		void UpdateCombinedMat(const bool bSpotShadowPass);
		void UploadCombinedMat(ID3D12GraphicsCommandList* const pCommandlist);
		void UploadMatBuffer(ID3D12GraphicsCommandList* const pCommandlist);
		void UploadProjMat(ID3D12GraphicsCommandList* const pCommandlist, const bool bSsao);
		void UploadViewMat(ID3D12GraphicsCommandList* const pCommandlist);

		MatrixHandler12(ID3D12Device* const pDevice, ID3D12GraphicsCommandList* const pCommandlist, HeapManager* const pHeapManager,
			XMMATRIX* const pViewMat, XMMATRIX* const pProjMat);
		~MatrixHandler12();

	private:
		bool bDirty;
		MatrixBuffer m_matrixBuffer;

		XMMATRIX m_extraMat;
		XMMATRIX m_invWorldMat;
		XMMATRIX m_invWVMat;
		XMMATRIX m_lightSpaceMat;
		XMMATRIX m_projMat;
		XMMATRIX m_viewMat;
		XMMATRIX m_worldMat;
		XMMATRIX m_wvMat;
		XMMATRIX m_wvpMat;
		XMMATRIX m_wvpTransposedMat;
		XMMATRIX m_wvpLitMat;

		D3D12_GPU_DESCRIPTOR_HANDLE m_combinedMatCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_matCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_projMatCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_viewMatCbv;
	
		UINT8* m_pCombinedMatCbBegin;
		UINT8* m_pMatricesCbBegin;
		UINT8* m_pProjMatCbBegin;
		UINT8* m_pViewMatCbBegin;
	};
}