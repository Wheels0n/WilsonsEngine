#ifndef ENTITY_H
#define ENTITY_H

#include <Windows.h>
#include <DirectXMath.h>
#include <string>
class CEntity
{
public:
	CEntity(std::string, DirectX::XMMATRIX*);
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
private:
	std::string m_type;
	DirectX::XMMATRIX* m_pWorldMat;
};

#endif // !ENTITY_H

