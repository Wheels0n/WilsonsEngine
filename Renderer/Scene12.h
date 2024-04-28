#pragma once
#include "typedef.h"
#include "../ImGui/imgui.h"
#include "D3D12.h"
#include "Entity12.h"
#include "Camera12.h"

namespace wilson
{
	class Scene12
	{
	public:

		void AddLightEntity(Light12* const, const std::string, const UINT, const UINT);
		void AddMeshEntity(Object12* const, const UINT, const UINT);
		void Draw();
		void DrawVec3Control(const std::string&, float* const);
		inline UINT GetEntitySize() const
		{
			return m_entites.size();
		}
		inline Scene12* GetScene() const
		{
			return m_pSceneHandler;
		}
		inline void SetCam(Camera12* const pCam)
		{
			m_pCam = pCam;
		}
		inline void SetSceneName(const std::string name)
		{
			m_name = name;
		}
		void Pick(const float, const float, const int, const int);

		Scene12() = default;
		Scene12(D3D12* const);
		Scene12(const Scene12&) = default;
		~Scene12();
	private:
		void DrawCubeLightControl(Light12* const);
		void DrawLightControl(Light12* const);
		void DrawSpotLightControl(Light12* const);
		bool RaySphereIntersect(const XMFLOAT3, const XMFLOAT3, const float, float* const);
		void RemoveEntity(const int);
		void RemoveLight(const int, const int, Light12* const);
		void RemoveObject(const int, const int);
		void RemoveSelectedMesh(const int, const int);
		void UnselectMesh();
	private:
		std::unordered_map<std::string, int> m_nEntity;
		std::vector<Entity12*> m_entites;
		std::string m_name;
		std::string m_popUpID;

		bool m_bObject;

		Camera12* m_pCam;
		D3D12* m_pD3D12;
		D3D12_GPU_DESCRIPTOR_HANDLE* m_pShadowSrv = nullptr;
		D3D12_GPU_DESCRIPTOR_HANDLE* m_pTextureSrv = nullptr;
		ShadowMap12* m_pShadow;
		Scene12* m_pSceneHandler;
		void* m_pSelectedEntity;
	};
}

