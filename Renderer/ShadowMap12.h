#pragma once
#include <d3d12.h>
#include "typedef.h"
namespace wilson
{	
	class HeapManager;
	class ShadowMap12
	{

	public:
		void BindCubeDsv(ID3D12GraphicsCommandList* const pCommandlist, const UINT i);
		inline void BindCubeSrv(ID3D12GraphicsCommandList* const pCommandlist)
		{
			pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psCubeShadow), m_cubeSrvs[0]);
		};
		void BindDirDsv(ID3D12GraphicsCommandList* const pCommandlist, const UINT i);
		inline void BindDirSrv(ID3D12GraphicsCommandList* const pCommandlist)
		{
			pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psDirShadow), m_dirSrvs[0]);
		};
		void BindSpotDsv(ID3D12GraphicsCommandList* const pCommandlist, const UINT i);
		inline void BindSpotSrv(ID3D12GraphicsCommandList* const pCommandlist)
		{
			pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psSpotShadow), m_spotSrvs[0]);
		};
		void ClearDsv(ID3D12GraphicsCommandList* const pCommandlist, const UINT litCounts[]);
		void ClearRtv(ID3D12GraphicsCommandList* const pCommandlist, const UINT litCounts[]);
		D3D12_GPU_DESCRIPTOR_HANDLE* GetCubeDebugSrv(ID3D12GraphicsCommandList* const pCommandlist, const UINT i, const UINT face);
		inline D3D12_GPU_DESCRIPTOR_HANDLE& GetCubeShadowSamplerView()
		{
			return m_cubeShadowSSV;
		}
		D3D12_GPU_DESCRIPTOR_HANDLE* GetDirDebugSrv(ID3D12GraphicsCommandList* const pCommandlist, const UINT i, const UINT lod);
		inline D3D12_GPU_DESCRIPTOR_HANDLE& GetDirShadowSamplerView()
		{
			return m_dirShadowSSV;
		}
		inline D3D12_RECT* GetScissorRect()
		{
			return &m_rect;
		}
		D3D12_GPU_DESCRIPTOR_HANDLE* GetSpotDebugSrv(ID3D12GraphicsCommandList* const pCommandlist, const UINT i);
		inline D3D12_VIEWPORT* GetViewport()
		{
			return &m_viewport;
		}
		void SetResourceBarrier(ID3D12GraphicsCommandList* const, const UINT litCnts[],
			const D3D12_RESOURCE_STATES beforeState, const D3D12_RESOURCE_STATES afterState, const bool bRTV);

		ShadowMap12() = default;
		ShadowMap12(ID3D12Device* const pDevice, ID3D12GraphicsCommandList* const pCommandList,
			HeapManager* const pHeapManager,
			const UINT width, const UINT height, const UINT cascadeLevel,
			const UINT dirLightCap, const UINT pntLightCap, const UINT spotLightCap);
		~ShadowMap12();

	private:
		UINT m_width;
		UINT m_height;
		FLOAT m_clear[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		//D3D12
		std::vector<ID3D12Resource*> m_cubeTexes;
		std::vector<ID3D12Resource*> m_cubeDebugTexes;
		std::vector<ID3D12Resource*> m_debugTexes;
		std::vector<ID3D12Resource*> m_dirTexes;
		std::vector<ID3D12Resource*> m_dirDebugTexes;
		std::vector<ID3D12Resource*> m_spotTexes;
		std::vector<ID3D12Resource*> m_spotDebugTexes;

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_cubeDebugRtvs;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_dirDebugRtvs;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_spotDebugRtvs;

		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_cubeSrvs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_cubeDebugSrvs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_debugSrvs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_dirSrvs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_dirDebugSrvs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_spotSrvs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_spotDebugSrvs;

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_cubeDsvs;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_dirDsvs;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_spotDsvs;

		D3D12_GPU_DESCRIPTOR_HANDLE m_cubeShadowSSV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_dirShadowSSV;

		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_rect = { 0, };
	};
};