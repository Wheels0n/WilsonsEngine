#include "Entity.h"

namespace wilson
{
	Entity::Entity(std::string name, ModelGroup* pModelGroup)
	{
		m_pModelGroup = pModelGroup;
		m_Name = name;
	}
}