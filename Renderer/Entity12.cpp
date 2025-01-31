#include "Entity12.h"

namespace wilson
{

	Entity12::Entity12(const string& name, const UINT entityIdx, shared_ptr<Mesh12>pMesh, const UINT meshIdx)
		:m_hasMesh(true), m_hasLight(false), m_pMesh(pMesh), m_name(name), m_pLight(nullptr),
		 m_meshIdx(meshIdx), m_entityIdx(entityIdx), m_lightIdx(_INVALID_INDEX)
	{
	}
	Entity12::Entity12(const string& name, const UINT entityIdx, shared_ptr<Light12>pLight, const UINT lightIdx)
		:m_hasMesh(false), m_hasLight(true), m_pMesh(nullptr), m_name(name), m_pLight(pLight),
		m_meshIdx(_INVALID_INDEX), m_entityIdx(entityIdx), m_lightIdx(lightIdx)
	{
	}

	void Entity12::DecreaseEntityIndex()
	{
		--m_entityIdx;
	}
	void Entity12::DecreaseLightIndex()
	{
		--m_lightIdx;
	}
	void Entity12::DecreaseMeshIndex()
	{
		--m_meshIdx;
	}

	BOOL Entity12::HasMesh()
	{
		return m_hasMesh;
	}
	BOOL Entity12::HasLight()
	{
		return m_hasLight;
	}

	int Entity12::GetEntityIndex()
	{
		return m_entityIdx;
	}
	int Entity12::GetMeshIndex()
	{
		return m_meshIdx;
	}
	int Entity12::GetLightIndex()
	{
		return m_lightIdx;
	}
	string& Entity12::GetName()
	{
		return m_name;
	}
	shared_ptr<Light12> Entity12::GetLight()
	{
		return m_pLight;
	}
	shared_ptr<Mesh12> Entity12::GetMesh()
	{
		return m_pMesh;
	}
}