
#pragma once
#include "typedef.h"
#include "../ImGui/imgui.h"
#include "D3D12.h"
#include "Entity12.h"
#include "Camera12.h"

namespace wilson
{
	using namespace std;
	using namespace DirectX;
	using namespace Microsoft::WRL;

	
	class Scene12 : public enable_shared_from_this<Scene12>
	{
	public:

		void AddLightEntity(shared_ptr<Light12>, const string&, const UINT, const UINT);
		void AddMeshEntity(shared_ptr<MeshGroup12>, const UINT, const UINT);
		void Draw();
		
		UINT				GetNumEntity();
		UINT				GetNumMesh();
		UINT				GetNumLight(const eLIGHT_TYPE);
		unordered_set<shared_ptr<Mesh12>>&		GetTotalMeshes()
		{
			return m_pTotalMeshes;
		}
		shared_ptr<ShadowMap12>					GetShadowMap() 
		{
			return m_pShadowMap;
		}
		shared_ptr<LightBuffer12> GetLightCb()
		{
			return m_pLightCb;
		}


		void	AddObject(shared_ptr<MeshGroup12>);
		void	AddLight(shared_ptr<Light12>pLight);

		void SetCam(shared_ptr<Camera12> pCam);
		void SetSceneName(const string& name);
		
		void Pick(const float, const float, const int, const int);

		Scene12(shared_ptr<D3D12>);
		Scene12(const Scene12&) = default;
		~Scene12();
	private:
		
		void DrawLightControl(shared_ptr<Light12>);

		bool RaySphereIntersect(const XMFLOAT3, const XMFLOAT3, const float, float* const);
		void RemoveEntity(const int);
		void RemoveMesh(shared_ptr<Mesh12>);
		void RemoveObject(shared_ptr<MeshGroup12>);
		void RemoveLight(const int, shared_ptr<Light12>);
		void UnselectEntity();
	private:
		unordered_map<string, int> m_nEntity;
		vector<shared_ptr<Entity12>> m_entites;
		string m_name;
		string m_popUpID;

		shared_ptr<LightBuffer12>	m_pLightCb;
		shared_ptr<Camera12>		m_pCam;
		shared_ptr<ShadowMap12>		m_pShadowMap;

		D3D12_GPU_DESCRIPTOR_HANDLE* m_pShadowSrv = nullptr;
		D3D12_GPU_DESCRIPTOR_HANDLE* m_pTextureSrv = nullptr;
		shared_ptr<Entity12>		m_pSelectedEntity;

		vector<shared_ptr<MeshGroup12>>			m_objects;
		unordered_set<shared_ptr<Mesh12>>		m_pTotalMeshes;
	};
}

