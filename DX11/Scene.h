#ifndef SCENE_H
#define SCENE_H

#include "../ImGui/imgui.h"
#include "D3D11.h"
#include "Entity.h"
#include "camera.h"
#include <Windows.h>
#include <unordered_map>
#include <vector>

class CScene
{
public:
	CScene();
	CScene(const CScene&)= default;
	~CScene();

	void Init(CD3D11*);
	void AddEntity(CModel*);
	void Draw();
	void DrawVec3Control(const std::string& label, float* vals);

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

	CD3D11* m_pCD3D11;
	CEntity* m_pSelectionETT;
	CScene* SceneHandler ;
	CCamera* m_pCCam ;

	bool RaySphereIntersect(XMFLOAT3, XMFLOAT3, float, float*);
	void RemoveENTT(int);
};


#endif // !SCENE_H

