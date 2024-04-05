#include "Entity12.h"

namespace wilson
{

	Entity12::Entity12(std::string name, UINT entityIdx, Object* pModelGroup, UINT modelIdx)
	{
		m_isObject = true;
		m_pObject = pModelGroup;
		m_Name = name;
		m_pLight = nullptr;
		m_objectIdx = modelIdx;
		m_entityIdx = entityIdx;
		m_lightIdx = -1;
	}
	Entity12::Entity12(std::string name, UINT entityIdx, Light12* pLight, UINT lightIdx)
	{
		m_isObject = false;
		m_pObject = nullptr;
		m_Name = name;
		m_pLight = pLight;
		m_lightIdx = lightIdx;
		m_entityIdx = entityIdx;
		m_objectIdx = -1;
	}
}