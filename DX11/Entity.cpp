#include "Entity.h"

namespace wilson
{
	Entity::Entity(std::string type, Model* pModel)
	{
		m_pModel = pModel;
		m_type = type;
	}
}