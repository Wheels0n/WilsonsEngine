#ifndef ENTITY_H
#define ENTITY_H

#include <Windows.h>
#include <DirectXMath.h>
#include <string>
#include "Model.h"
class CEntity
{
public:
	CEntity(std::string, DirectX::XMMATRIX*, CModel*);
	CEntity(const CEntity&) = default;
	~CEntity() = default;

	std::string* GetType()
	{
		return &m_type;
	};

	DirectX::XMMATRIX* GetMat()
	{
		return m_pWorldMat;
	};

	CModel* GetModel() const
	{
		return m_pModel;
	}
private:
	CModel* m_pModel;
	std::string m_type;
	DirectX::XMMATRIX* m_pWorldMat;
};

#endif // !ENTITY_H

