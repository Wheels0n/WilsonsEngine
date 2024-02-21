#pragma once

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include "typedef.h"
using namespace DirectX;

namespace wilson
{
	class DescriptorHeapManager;
	class MatBuffer12
	{
	public:
		void UpdateMatBuffer(ID3D12GraphicsCommandList* pCommandlist, bool bSpotShadowPass);
		void UploadProjMat(ID3D12GraphicsCommandList* pCommandlist);

		inline void SetProjMatrix(XMMATRIX* projMat)
		{
			m_projMat = *projMat;
		}
		inline void SetViewMatrix(XMMATRIX* viewMatrix)
		{
			m_viewMat = *viewMatrix;
		}
		inline void SetWorldMatrix(XMMATRIX* worldMatrix)
		{
			m_worldMat = *worldMatrix;
		}
		inline void SetInvWorldMatrix(XMMATRIX* invWorldMatrix)
		{
			m_invWorldMat = *invWorldMatrix;
		}
		inline void SetLightSpaceMatrix(XMMATRIX* plightSpaceMat)
		{
			m_lightSpaceMat = *plightSpaceMat;
		}

		MatBuffer12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandlist, DescriptorHeapManager* pDescriptorHeapManager,
			XMMATRIX* pViewMat, XMMATRIX* pProjMat);
		~MatBuffer12();

	private:

		XMMATRIX m_worldMat;
		XMMATRIX m_projMat;
		XMMATRIX m_viewMat;
		XMMATRIX m_lightSpaceMat;
		XMMATRIX m_invWorldMat;
		XMMATRIX m_extraMat;

		ID3D12Resource* m_pMat12Cb;
		ID3D12Resource* m_pProjMat12Cb;
		D3D12_GPU_DESCRIPTOR_HANDLE m_projMatCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_matBufferCBV;
	};
}