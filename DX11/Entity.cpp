#include "Entity.h"

CEntity::CEntity(std::string type, CModel* pModel)
{  
	m_pModel = pModel;
	m_type = type;
}

std::string* CEntity::GetType()
{
	return &m_type;
};

CModel* CEntity::GetModel() const
{
	return m_pModel;
}