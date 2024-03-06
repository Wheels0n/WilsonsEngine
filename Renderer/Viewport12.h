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
		void Draw();
		void Resize();
		bool CheckRange(int, int);

		inline float GetNDCX(int);
		inline float GetNDCY(int);
		inline int GetWidth() const
		{
			return m_width;
		};
		inline int GetHeight() const
		{
			return m_height;
		};
		inline int GetX() const
		{
			return m_left;
		};
		inline int GetY() const
		{
			return m_top;
		};

		Viewport12() = default;
		Viewport12(D3D12*, Scene12*);
		Viewport12(const Viewport12&) = delete;
		~Viewport12();
	private:
		XMVECTOR CalEntityPos();
	private:
		bool m_IsFocused;

		UINT m_width;
		UINT m_height;
		UINT m_GbufferCount;
		int m_left;
		int m_top;

		D3D12* m_pD3D12;
		DescriptorHeapManager* m_pDescripotrHeapManager;
		Camera12* m_pCam;
		Scene12* m_pScene12;
		Importer12* m_pImporter12;

		D3D12_GPU_DESCRIPTOR_HANDLE* m_pFinalSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE* m_pSceneDepthSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE* m_pSSAOBlurredSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE* m_pGbufferSRVs;

		ID3D12Device* m_pDevice;
	};
}

