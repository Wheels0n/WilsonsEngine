#pragma once

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

struct MatrixBuffer
{
	XMMATRIX m_worldMat;
	XMMATRIX m_viewMat;
	XMMATRIX m_projMat;
	XMMATRIX m_extraMat;
};
class MatBuffer
{
public:
	void Update(ID3D11DeviceContext* pContext, bool bSpotShadowPass);
	void UploadProjMat(ID3D11DeviceContext* pContext);

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
	inline ID3D11Buffer* GetMatrixBuffer() const
	{
		return m_pMatBuffer;
	};

	MatBuffer(ID3D11Device*, ID3D11DeviceContext*, XMMATRIX*, XMMATRIX*);
	~MatBuffer();

private:
	
	XMMATRIX m_worldMat;
	XMMATRIX m_projMat;
	XMMATRIX m_viewMat;
	XMMATRIX m_lightSpaceMat;
	XMMATRIX m_invWorldMat;
	XMMATRIX m_extraMat;

	ID3D11Buffer* m_pMatBuffer;
	ID3D11Buffer* m_pProjMatBuffer;
};