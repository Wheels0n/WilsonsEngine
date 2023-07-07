#include "Entity.h"

namespace wilson
{
	Entity::Entity(std::string name, ModelGroup* pModelGroup)
	{
		m_isModel = true;
		m_pModelGroup = pModelGroup;
		m_Name = name;
		m_pLight=nullptr;
	}
	Entity::Entity(std::string name, Light* pLight)
	{
		m_isModel = false;
		m_pModelGroup = nullptr;
		m_Name = name;
		m_pLight=pLight;
	}
}