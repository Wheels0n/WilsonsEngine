#include "Entity.h"

CEntity::CEntity(std::string type, DirectX::XMMATRIX *pWorldMat, CModel* pModel)
{  
	m_pModel = pModel;
	m_type = type;
	m_pWorldMat = pWorldMat;
}
