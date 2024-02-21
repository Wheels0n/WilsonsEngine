#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "Camera.h"
#include "typedef.h"

namespace wilson
{   

	class Light
	{
	public:
		eLIGHT_TYPE virtual GetType() { return eLIGHT_TYPE::PNT; };
		inline UINT GetLightIndex() const
		{
			return m_entityIdx;
		}
		inline DirectX::XMFLOAT3* GetPos()
		{
			return &m_position;
		}
		inline DirectX::XMFLOAT3* GetDir()
		{
			return &m_direction;
		}
		inline DirectX::XMVECTOR* GetAmbient()
		{
			return &m_ambient;
		}
		inline DirectX::XMVECTOR* GetDiffuse()
		{
			return &m_diffuse;
		}
		inline DirectX::XMVECTOR* GetSpecular()
		{
			return &m_specular;
		}

		void virtual UpdateProperty() {};
		Light(UINT);
		virtual ~Light();
	protected:
		ID3D11Buffer* m_pLightBuffer;

		DirectX::XMVECTOR m_ambient;
		DirectX::XMVECTOR m_diffuse;
		DirectX::XMVECTOR m_specular;
		DirectX::XMFLOAT3 m_position;
		DirectX::XMFLOAT3 m_direction;

		UINT m_entityIdx;
	};
}