#include "Light12.h"

namespace wilson
{
	Light12::Light12(const UINT idx)
	{
		m_ambient = m_ambient = DirectX::XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f);
		m_diffuse = m_ambient;
		m_specular = m_ambient;
		m_position = DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f);
		m_direction = DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f);;

		m_entityIdx = idx;
	}
	Light12::~Light12()
	{

	}

}