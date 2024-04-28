#include "Entity11.h"

namespace wilson
{
	Entity11::Entity11(const  std::string name, const UINT idx, Object11* const  pObject, const UINT obejctIdx)
	{
		m_bObject = true;
		m_pObject = pObject;
		m_name = name;
		m_pLight=nullptr;
		m_objectIdx = obejctIdx;
		m_entityIdx = idx;
		m_lightIdx = _UNSELECT;
	}
	
	Entity11::Entity11(const std::string name, const UINT idx, Light11* const  pLight, const UINT lightIdx)
	{	
		m_bObject = false;
		m_pObject = nullptr;
		m_name = name;
		m_pLight=pLight;
		m_lightIdx = lightIdx;
		m_entityIdx = idx;
		m_objectIdx = _UNSELECT;
	}
}