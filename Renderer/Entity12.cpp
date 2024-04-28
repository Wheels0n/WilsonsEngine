#include "Entity12.h"

namespace wilson
{

	Entity12::Entity12(const std::string name, const UINT entityIdx, Object12* const pObject, const UINT meshIdx)
	{
		m_bObject = true;
		m_pObject = pObject;
		m_name = name;
		m_pLight = nullptr;
		m_objectIdx = meshIdx;
		m_entityIdx = entityIdx;
		m_lightIdx = -1;
	}
	Entity12::Entity12(const std::string name, const UINT entityIdx, Light12* const pLight, const UINT lightIdx)
	{
		m_bObject = false;
		m_pObject = nullptr;
		m_name = name;
		m_pLight = pLight;
		m_lightIdx = lightIdx;
		m_entityIdx = entityIdx;
		m_objectIdx = -1;
	}
}