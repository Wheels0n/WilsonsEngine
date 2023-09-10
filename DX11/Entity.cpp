#include "Entity.h"

namespace wilson
{
	Entity::Entity(std::string name, UINT idx, ModelGroup* pModelGroup, UINT modelIdx)
	{
		m_isModel = true;
		m_pModelGroup = pModelGroup;
		m_Name = name;
		m_pLight=nullptr;
		m_modelIdx = modelIdx;
		m_idx = idx;
		m_lightIdx = -1;
	}
	Entity::Entity(std::string name, UINT idx, Light* pLight, UINT lightIdx)
	{	
		m_isModel = false;
		m_pModelGroup = nullptr;
		m_Name = name;
		m_pLight=pLight;
		m_lightIdx = lightIdx;
		m_idx = idx;
		m_modelIdx = -1;
	}
}