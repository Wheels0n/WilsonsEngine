#pragma once 

#include"D3D12.h"
#include"FPS.h"
#include"Camera12.h"
#include"Light12.h"

namespace wilson
{	
	class HeapManager;
	class Settings12
	{
	public:
		void Draw();

		Settings12() = default;
		Settings12(D3D12* const pD3D12);
		Settings12(const Settings12&) = delete;
		~Settings12();
	private:
		//D3D12 
		ID3D12Resource* m_pCubeLitIconTex;
		ID3D12Resource* m_pCubeLitIconUploadCb;
		ID3D12Resource* m_pDirLitIconTex;
		ID3D12Resource* m_pDirLitIconUploadCb;
		ID3D12Resource* m_pSpotLitIconTex;
		ID3D12Resource* m_pSpotLitIconUploadCb;

		D3D12_GPU_DESCRIPTOR_HANDLE m_cubeLitIconSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_dirLitIconSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_spotLitIconSrv;

		Camera12* m_pCam;
		D3D12* m_pD3D12;
		Frustum12* m_pFrustum;
		FPS m_FPS;
		Light12* m_pLight;

		BOOL* m_pbHeightOn;
		float* m_pExposure;
		float* m_pHeightScale;
		float* m_pSsaoBias;
		float* m_pSsaoRadius;
		UINT* m_pnSsaoSample;

		XMFLOAT4 m_prevPos;
		XMFLOAT3 m_prevAngleFloat;
		float m_prevFarZ;
		float m_prevNearZ;
	};
}

