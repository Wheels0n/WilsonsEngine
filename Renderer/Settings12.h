#pragma once 

#include"D3D12.h"
#include"FPS.h"
#include"Camera12.h"
#include"Light12.h"

namespace wilson
{	
	class DescriptorHeapManager;
	class Settings12
	{
	public:
		void Draw();

		Settings12() = default;
		Settings12(D3D12* pD3D12);
		Settings12(const Settings12&) = delete;
		~Settings12();
	private:
		//D3D12 
		ID3D12Resource* m_pDirLitIcon12Tex;
		ID3D12Resource* m_pCubeLitIcon12Tex;
		ID3D12Resource* m_pSpotLitIcon12Tex;
		ID3D12Resource* m_pDirLitIcon12UploadCB;
		ID3D12Resource* m_pPntLitIcon12UploadCB;
		ID3D12Resource* m_pSpotLitIcon12UploadCB;

		D3D12_GPU_DESCRIPTOR_HANDLE m_dirLitIcon12SRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_pntLitIcon12SRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_spotLitIcon12SRV;

		FPS m_FPS;
		D3D12* m_pD3D12;
		Light12* m_pLight;
		Camera12* m_pCam;
		Frustum12* m_pFrustum;

		BOOL* m_pHeightOnOFF;
		float* m_pExposure;
		float* m_pHeightScale;

		XMFLOAT3 m_prevPos;
		XMFLOAT3 m_prevAngleFloat;
		float m_prevFarZ;
		float m_prevNearZ;
	};
}

