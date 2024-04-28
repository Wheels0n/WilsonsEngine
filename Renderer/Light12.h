#pragma once

#include <dxgi.h>
#include <d3d12.h>

#include "Camera12.h"
#include "typedef.h"

namespace wilson
{

	class Light12
	{
	public:
		inline DirectX::XMVECTOR* GetAmbient()
		{
			return &m_ambient;
		}
		inline DirectX::XMVECTOR* GetDiffuse()
		{
			return &m_diffuse;
		}
		inline DirectX::XMFLOAT3* GetDir()
		{
			return &m_direction;
		}
		inline UINT GetLightIndex() const
		{
			return m_entityIdx;
		}
		inline DirectX::XMFLOAT3* GetPos()
		{
			return &m_position;
		}
		inline DirectX::XMVECTOR* GetSpecular()
		{
			return &m_specular;
		}
		eLIGHT_TYPE virtual GetType() { return eLIGHT_TYPE::CUBE; };
		void virtual UpdateProperty() {};

		Light12(const UINT);
		virtual ~Light12();
	protected:

		DirectX::XMVECTOR m_ambient;
		DirectX::XMVECTOR m_diffuse;
		DirectX::XMVECTOR m_specular;

		DirectX::XMFLOAT3 m_direction;
		DirectX::XMFLOAT3 m_position;

		UINT m_entityIdx;
	};
}