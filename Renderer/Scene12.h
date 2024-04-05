#pragma once

#include <Windows.h>
#include <unordered_map>
#include <vector>
#include "../ImGui/imgui.h"
#include "D3D12.h"
#include "Entity12.h"
#include "Camera12.h"

namespace wilson
{
	class Scene12
	{
	public:

		void AddModelEntity(Object* pModelGroup, UINT idx, UINT modelIdx);
		void AddLightEntity(Light12* pLight, std::string type, UINT idx, UINT lightIdx);
		void Draw();
		void DrawVec3Control(const std::string& label, float* vals);
		void Pick(float, float, int, int);

		inline void SetCam(Camera12* pCam)
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
		inline Scene12* GetScene() const
		{
			return sceneHandler;
		}

		Scene12() = default;
		Scene12(D3D12* pD3D11);
		Scene12(const Scene12&) = default;
		~Scene12();
	private:
		void DrawLightControl(Light12*);
		void DrawCubeLightControl(Light12*);
		void DrawSpotLightControl(Light12*);
		bool RaySphereIntersect(XMFLOAT3, XMFLOAT3, float, float*);
		void RemoveSelectedModel(int, int);
		void RemoveObject(int, int);
		void RemoveLight(int, int, Light12*);
		void RemoveEntity(int);
		void UnselectModel();
	private:
		std::unordered_map<std::string, int> m_entityCnt;
		std::vector<Entity12*> m_entites;
		std::string m_name;
		std::string m_popUpID;

		D3D12_GPU_DESCRIPTOR_HANDLE* m_pShadowSrv = nullptr;
		D3D12_GPU_DESCRIPTOR_HANDLE* m_pTextureSrv = nullptr;
		bool m_isObject;
		D3D12* m_pD3D12;
		void* m_pSelectedEntity;
		Camera12* m_pCam;
		ShadowMap12* m_pShadow;
		Scene12* sceneHandler;
	};
}

