#include "Entity.h"

CEntity::CEntity(std::string type, DirectX::XMMATRIX *pWorldMat)
{
	m_type = type;
	m_pWorldMat = pWorldMat;
}
