#ifndef SCENE_H
#define SCENE_H

#include "../ImGui/imgui.h"
#include "Entity.h"
#include <Windows.h>
#include <vector>

class CScene
{
public:
	CScene() = default;
	CScene(const CScene&)= default;
	~CScene();

	void AddEntity(std::string, DirectX::XMMATRIX*);
	void Draw();

	void SetSceneName(std::string name)
	{
		m_name= name;
	}

	CScene* GetScene()
	{
		return SceneHandler;
	}
private:
	std::vector<CEntity*> m_entites;
	std::string m_name;
	CEntity* m_pSelectionETT = nullptr;
	CScene* SceneHandler = this;
};


#endif // !SCENE_H

