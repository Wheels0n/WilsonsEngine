#ifndef ENTITY_H
#define ENTITY_H

#include <Windows.h>
#include <DirectXMath.h>
#include <string>
#include "Model.h"
class CEntity
{
public:
	CEntity(std::string, CModel*);
	CEntity(const CEntity&) = default;
	~CEntity() = default;

	std::string* GetType();
	CModel* GetModel() const;
private:
	CModel* m_pModel;
	std::string m_type;
};

#endif // !ENTITY_H

