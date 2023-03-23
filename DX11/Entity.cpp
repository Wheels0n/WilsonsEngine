#include "Entity.h"

CEntity::CEntity(std::string type, Model* pModel)
{  
	m_pModel = pModel;
	m_type = type;
}


std::string* CEntity::GetType()
{
	return &m_type;
};

Model* CEntity::GetModel() const
{
	return m_pModel;
}