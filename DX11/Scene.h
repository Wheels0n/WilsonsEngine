#ifndef SCENE_H
#define SCENE_H

#include "../ImGui/imgui.h"
#include "Entity.h"
#include "camera.h"
#include <Windows.h>
#include <unordered_map>
#include <vector>

class CScene
{
public:
	CScene() = default;
	CScene(const CScene&)= default;
	~CScene();

	void AddEntity(std::string, DirectX::XMMATRIX*, CModel*);
	void Draw();

	void SetCam(CCamera* pCam)
	{
		m_pCCam = pCam;
	}

	void SetSceneName(std::string name)
	{
		m_name= name;
	}

	CScene* GetScene() const
	{
		return SceneHandler;
	}

	CEntity* GetSelectedENTT() const
	{
		return  m_pSelectionETT;
	};

	void Pick(int, int, int, int);
private:
	std::unordered_map<std::string, int> m_entityCnt;
	std::vector<CEntity*> m_entites;
	std::string m_name;
	CEntity* m_pSelectionETT = nullptr;
	CScene* SceneHandler = this;
	CCamera* m_pCCam = nullptr;

	bool RaySphereIntersect(XMFLOAT3, XMFLOAT3, float);
};


#endif // !SCENE_H

