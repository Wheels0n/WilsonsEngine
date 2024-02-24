#pragma once
#include <d3d12.h>
#include <vector>
#include "typedef.h"
namespace wilson
{	
	class DescriptorHeapManager;
	class ShadowMap12
	{

	public:
		void BindDirDSV(ID3D12GraphicsCommandList* pCommandlist, const UINT i);
		void BindCubeDSV(ID3D12GraphicsCommandList* pCommandlist, const UINT i);
		void BindSpotDSV(ID3D12GraphicsCommandList* pCommandlist, const UINT i);
		void ClearDSV(ID3D12GraphicsCommandList* pCommandlist, UINT litCounts[]);
		void ClearRTV(ID3D12GraphicsCommandList* pCommandlist, UINT litCounts[]);
		inline void BindDirSRV(ID3D12GraphicsCommandList* pCommandlist)
		{
			pCommandlist->SetGraphicsRootDescriptorTable(ePbrLight_ePsDirShadow, m_dir12SRVs[0]);
		};
		inline void BindCubeSRV(ID3D12GraphicsCommandList* pCommandlist)
		{
			pCommandlist->SetGraphicsRootDescriptorTable(ePbrLight_ePsCubeShadow, m_cube12SRVs[0]);
		};
		inline void BindSpotSRV(ID3D12GraphicsCommandList* pCommandlist)
		{
			pCommandlist->SetGraphicsRootDescriptorTable(ePbrLight_ePsSpotShadow, m_spot12SRVs[0]);
		};
		void SetResourceBarrier(ID3D12GraphicsCommandList*, D3D12_RESOURCE_BARRIER, bool bRTV);
		D3D12_GPU_DESCRIPTOR_HANDLE* GetDirDebugSRV(ID3D12GraphicsCommandList* pCommandlist, UINT i, UINT lod);
		D3D12_GPU_DESCRIPTOR_HANDLE* GetCubeDebugSRV(ID3D12GraphicsCommandList* pCommandlist, UINT i, UINT face);
		D3D12_GPU_DESCRIPTOR_HANDLE* GetSpotDebugSRV(ID3D12GraphicsCommandList* pCommandlist, UINT i);

		
		inline D3D12_GPU_DESCRIPTOR_HANDLE& GetDirShadowSamplerView()
		{
			return m_dirShadowSSV;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE& GetCubeShadowSamplerView()
		{
			return m_cubeShadowSSV;
		}
	
		inline D3D12_VIEWPORT* GetViewport12()
		{
			return &m_viewport12;
		}

		inline D3D12_RECT* GetScissorRect()
		{
			return &m_rect;
		}
		ShadowMap12() = default;
		ShadowMap12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList,
			DescriptorHeapManager* pDescriptorHeapManager,
			const UINT width, const UINT height, const UINT cascadeLevel,
			const UINT dirLightCap, const UINT pntLightCap, const UINT spotLightCap);
		~ShadowMap12();

	private:
		UINT m_width;
		UINT m_height;
		FLOAT m_clear[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		//D3D12
		std::vector<ID3D12Resource*> m_dir12Tex;
		std::vector<ID3D12Resource*> m_dirDebug12Tex;
		std::vector<ID3D12Resource*> m_cube12Tex;
		std::vector<ID3D12Resource*> m_cubeDebug12Tex;
		std::vector<ID3D12Resource*> m_spot12Tex;
		std::vector<ID3D12Resource*> m_spotDebug12Tex;
		std::vector<ID3D12Resource*> m_debug12Tex;

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_dirDebug12RTVs;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_cubeDebug12RTVs;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_spotDebug12RTVs;

		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_dir12SRVs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_dirDebug12SRVs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_cube12SRVs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_cubeDebug12SRVs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_spot12SRVs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_spotDebug12SRVs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_debug12SRVs;

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_dir12DSVs;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_cube12DSVs;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_spot12DSVs;

		D3D12_GPU_DESCRIPTOR_HANDLE m_dirShadowSSV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_cubeShadowSSV;

		D3D12_VIEWPORT m_viewport12;
		D3D12_RECT m_rect = { 0, };
	};
};