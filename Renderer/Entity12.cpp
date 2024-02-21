#include "Entity12.h"

namespace wilson
{

	Entity12::Entity12(std::string name, UINT entityIdx, ModelGroup12* pModelGroup, UINT modelIdx)
	{
		m_isModel = true;
		m_pModelGroup12 = pModelGroup;
		m_Name = name;
		m_pLight = nullptr;
		m_modelGroupIdx = modelIdx;
		m_entityIdx = entityIdx;
		m_lightIdx = -1;
	}
	Entity12::Entity12(std::string name, UINT entityIdx, Light12* pLight, UINT lightIdx)
	{
		m_isModel = false;
		m_pModelGroup12 = nullptr;
		m_Name = name;
		m_pLight = pLight;
		m_lightIdx = lightIdx;
		m_entityIdx = entityIdx;
		m_modelGroupIdx = -1;
	}
}