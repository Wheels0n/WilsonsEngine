#pragma once

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include "typedef.h"
using namespace DirectX;

namespace wilson
{

	class MatBuffer11
	{
	public:
		inline ID3D11Buffer* GetMatrixBuffer() const
		{
			return m_pMatricesCb;
		};
		inline void SetInvWorldMatrix(XMMATRIX* const invWorldMatrix)
		{
			m_invWorldMat = *invWorldMatrix;
		}
		inline void SetLightSpaceMatrix(XMMATRIX* const plightSpaceMat)
		{
			m_lightSpaceMat = *plightSpaceMat;
		}
		inline void SetProjMatrix(XMMATRIX* const projMat)
		{
			m_projMat = *projMat;
		}
		inline void SetViewMatrix(XMMATRIX* const viewMatrix)
		{
			m_viewMat = *viewMatrix;
		}
		inline void SetWorldMatrix(XMMATRIX* const worldMatrix)
		{
			m_worldMat = *worldMatrix;
		}
		void UploadMatBuffer(ID3D11DeviceContext* const pContext, const bool bSpotShadowPass);
		void UploadProjMat(ID3D11DeviceContext* const pContext);

		MatBuffer11(ID3D11Device* const, ID3D11DeviceContext* const, XMMATRIX* const, XMMATRIX* const);
		~MatBuffer11();

	private:

		XMMATRIX m_extraMat;
		XMMATRIX m_invWorldMat;
		XMMATRIX m_lightSpaceMat;
		XMMATRIX m_worldMat;
		XMMATRIX m_projMat;
		XMMATRIX m_viewMat;

		ID3D11Buffer* m_pMatricesCb;
		ID3D11Buffer* m_pProjMatCb;

	};
}