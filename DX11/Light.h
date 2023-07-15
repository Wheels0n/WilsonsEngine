#ifndef LIGHT_H
#define LIGHT_H

#include <dxgi.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Camera.h"

namespace wilson
{   
	enum class ELIGHT_TYPE
	{
		DIR,
		PNT,
		SPT
	};

	class Light
	{
	public:
		bool virtual Init(ID3D11Device* pDevice);
		ELIGHT_TYPE virtual GetType() { return ELIGHT_TYPE::PNT; };

		inline DirectX::XMFLOAT3* GetPos()
		{
			return &m_position;
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
		Light();
		~Light();
	protected:
		ID3D11Buffer* m_pLightBuffer;
		DirectX::XMVECTOR m_ambient;
		DirectX::XMVECTOR m_diffuse;
		DirectX::XMVECTOR m_specular;
		DirectX::XMFLOAT3 m_position;

	};
}
#endif 
