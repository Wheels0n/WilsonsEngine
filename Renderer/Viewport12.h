#pragma once

#include <d3d12.h>

#include "../ImGui/imgui.h"
#include "D3D12.h"
#include "Scene12.h"
#include "Import12.h"
#include "typedef.h"
namespace wilson
{
	class Viewport12
	{
	public:
		bool CheckRange(const int, const int);
		void Draw();
		inline int GetHeight() const
		{
			return m_height;
		};
		inline float GetNDCX(const int);
		inline float GetNDCY(const int);
		inline int GetWidth() const
		{
			return m_width;
		};
		inline int GetX() const
		{
			return m_left;
		};
		inline int GetY() const
		{
			return m_top;
		};
		void Resize();

		Viewport12() = default;
		Viewport12(D3D12* const, Scene12* const);
		Viewport12(const Viewport12&) = delete;
		~Viewport12();
	private:
		XMVECTOR CalEntityPos();
	private:
		bool m_IsFocused;

		UINT m_width;
		UINT m_height;
		UINT m_nGbuffer;

		int m_left;
		int m_top;

		Camera12* m_pCam;
		D3D12* m_pD3D12;
		HeapManager* m_pHeapManager;
		Importer12* m_pImporter;
		Scene12* m_pScene;

		D3D12_GPU_DESCRIPTOR_HANDLE* m_pGbufferSrvs;
		D3D12_GPU_DESCRIPTOR_HANDLE* m_pPostProcessSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE* m_pSceneDepthSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE* m_pSsaoBlurredSrv;

		ID3D12Device* m_pDevice;
	};
}

