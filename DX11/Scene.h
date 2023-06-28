#ifndef SCENE_H
#define SCENE_H

#include "../ImGui/imgui.h"
#include "D3D11.h"
#include "Entity.h"
#include "Camera.h"
#include <Windows.h>
#include <unordered_map>
#include <vector>

namespace wilson
{
	class Scene
	{
	public:

		inline void Init(D3D11* pD3D11)
		{
			m_pD3D11 = pD3D11;
		}
		void AddEntity(ModelGroup*);
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
		inline Scene* GetScene() const
		{
			return sceneHandler;
		}

		Scene();
		Scene(const Scene&) = default;
		~Scene();
	private:
		bool RaySphereIntersect(XMFLOAT3, XMFLOAT3, float, float*);
		void RemoveSelectedModel(int, int);
		void RemoveModelGroup(int);
	private:
		std::unordered_map<std::string, int> m_entityCnt;
		std::vector<Entity*> m_entites;
		std::string m_name;

		D3D11* m_pD3D11;
		Model* m_pSelectedModel;
		Camera* m_pCam;
		Scene* sceneHandler;
	};
}
#endif // !SCENE_H

