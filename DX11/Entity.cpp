#include "Entity.h"

namespace wilson
{
	Entity::Entity(std::string name, UINT idx, ModelGroup* pModelGroup)
	{
		m_isModel = true;
		m_pModelGroup = pModelGroup;
		m_Name = name;
		m_pLight=nullptr;
		m_idx = idx;
	}
	Entity::Entity(std::string name, UINT idx, Light* pLight)
	{	
		m_isModel = false;
		m_pModelGroup = nullptr;
		m_Name = name;
		m_pLight=pLight;
		m_idx = idx;
	}
}