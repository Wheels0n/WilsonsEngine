#pragma once

#include "../ImGui/imgui.h"
#include "D3D11.h"
#include "Entity11.h"
#include "Camera11.h"

namespace wilson
{
	class Scene11
	{
	public:

		void AddLightEntity(Light11* const, const std::string, const UINT, const UINT);
		void AddMeshEntity(Object11* const, const UINT, const UINT);
		void Draw();
		void DrawVec3Control(const std::string&, float* const);
		inline UINT GetEntitySize() const
		{
			return m_entites.size();
		}
		inline Scene11* GetScene() const
		{
			return m_pSceneHandler;
		}
		inline void SetCam(Camera11* const pCam)
		{
			m_pCam = pCam;
		}
		inline void SetSceneName(const std::string name)
		{
			m_name = name;
		}
		void Pick(const float, const float, const int, const int);

		Scene11()=default;
		Scene11(D3D11* const);
		Scene11(const Scene11&) = default;
		~Scene11();
	private:
		void DrawCubeLightControl(Light11*const);
		void DrawLightControl(Light11* const);
		void DrawSpotLightControl(Light11* const);
		bool RaySphereIntersect(const XMFLOAT3, const XMFLOAT3, const float, float* const);
		void RemoveEntity(int);
		void RemoveLight(const int, const int, Light11* const);
		void RemoveObject(const int, const int);
		void RemoveSelectedMesh(const int, const int);
		void UnselectMesh();
	private:
		std::unordered_map<std::string, int> m_nEntity;
		std::vector<Entity11*> m_entites;
		std::string m_name;
		std::string m_popUpID;

		bool m_bObject;

		Camera11* m_pCam;
		D3D11* m_pD3D11;
		ID3D11ShaderResourceView* m_pShadowSrv;
		Scene11* m_pSceneHandler;
		ShadowMap* m_pShadow;
		void* m_pSelectedEntity;
	};
}

