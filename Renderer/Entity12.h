#pragma once
#include "typedef.h"
#include "Object12.h"
#include "DirectionalLight12.h"
#include "CubeLight12.h"
#include "SpotLight12.h"

namespace wilson
{
	using namespace std;
	
	class Entity12
	{
		public:
			void						DecreaseEntityIndex();
			void						DecreaseLightIndex();
			void						DecreaseMeshIndex();

			BOOL						HasMesh();
			BOOL						HasLight();

			int							GetEntityIndex();
			int							GetMeshIndex();
			int							GetLightIndex();
			string&						GetName();

			shared_ptr<Light12>			GetLight();
			shared_ptr<Mesh12>			GetMesh();

										Entity12(const string&, const UINT, shared_ptr<Mesh12>, const UINT);
										Entity12(const string&, const UINT, shared_ptr<Light12>, const UINT);
										Entity12(const Entity12&) = default;
										~Entity12() = default;
		private:
			BOOL						m_hasMesh;
			BOOL						m_hasLight;

			string						m_name;
		
			int							m_entityIdx;
			int							m_lightIdx;
			int							m_meshIdx;

			shared_ptr<Light12>			m_pLight;
			shared_ptr<Mesh12>			m_pMesh;
	};
}


