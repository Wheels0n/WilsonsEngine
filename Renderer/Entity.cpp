#include "Entity.h"

namespace wilson
{
	Entity::Entity(std::string name, UINT idx, ModelGroup* pModelGroup, UINT modelIdx)
	{
		m_isObject = true;
		m_pModelGroup = pModelGroup;
		m_Name = name;
		m_pLight=nullptr;
		m_objectIdx = modelIdx;
		m_entityIdx = idx;
		m_lightIdx = -1;
	}
	
	Entity::Entity(std::string name, UINT idx, Light* pLight, UINT lightIdx)
	{	
		m_isObject = false;
		m_pModelGroup = nullptr;
		m_Name = name;
		m_pLight=pLight;
		m_lightIdx = lightIdx;
		m_entityIdx = idx;
		m_objectIdx = -1;
	}
}