#pragma once

#include <Windows.h>
#include <unordered_map>
#include <vector>

#include "../ImGui/imgui.h"
#include "D3D11.h"
#include "Entity.h"
#include "Camera.h"

namespace wilson
{
	class Scene
	{
	public:

		inline void Init(D3D11* pD3D11)
		{
			m_pD3D11 = pD3D11;
			m_pShadow = pD3D11->GetShadowMap();
		}
		void AddModelEntity(ModelGroup* pModelGroup, UINT idx, UINT modelIdx);
		void AddLightEntity(Light* pLight, std::string type, UINT idx, UINT lightIdx);
		void Draw();
		void DrawVec3Control(const std::string& label, float* vals);
		void Pick(float, float, int, int);

		inline void SetCam(Camera* pCam)
		{
			m_pCam = pCam;
		}
		inline void SetSceneName(std::string name)
		{
			m_name = name;
		}
		inline UINT GetEntitySize() const
		{
			return m_entites.size();
		}
		inline Scene* GetScene() const
		{
			return sceneHandler;
		}

		Scene();
		Scene(const Scene&) = default;
		~Scene();
	private:
		void DrawLightControl(Light*);
		void DrawPointLightControl(Light*);
		void DrawSpotLightControl(Light*);
		bool RaySphereIntersect(XMFLOAT3, XMFLOAT3, float, float*);
		void RemoveSelectedModel(int, int, int);
		void RemoveModelGroup(int,int);
		void RemoveLight(int, int, Light*);
		void RemoveEntity(int);
		void UnselectModel();
	private:
		std::unordered_map<std::string, int> m_entityCnt;
		std::vector<Entity*> m_entites;
		std::string m_name;
		std::string m_popUpID;

		ID3D11ShaderResourceView* m_pShadowSRV = nullptr;
		bool m_isModel;
		D3D11* m_pD3D11;
		void* m_pSelectedEntity;
		Camera* m_pCam;
		ShadowMap* m_pShadow;
		Scene* sceneHandler;
	};
}

