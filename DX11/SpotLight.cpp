#include "SpotLight.h"

namespace wilson
{   

	bool SpotLight::Init(ID3D11Device* pDevice)
	{	
        D3D11_BUFFER_DESC lightCBD;
        lightCBD.Usage = D3D11_USAGE_DYNAMIC;
        lightCBD.ByteWidth = sizeof(SpotLightProperty);
        lightCBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        lightCBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        lightCBD.MiscFlags = 0;
        lightCBD.StructureByteStride = 0;

        HRESULT result = pDevice->CreateBuffer(&lightCBD, nullptr, &m_pLightBuffer);
        if (FAILED(result))
        {
            return false;
        }
        m_pLightBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            sizeof("SpotLight::m_pLightBuffer") - 1, "SpotLight::m_pLightBuffer");


        Light::Init(pDevice);
        m_direction =   DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);
        m_attenuation = DirectX::XMFLOAT3(1.0f, 0.09f, 0.032f);
        m_cutoff = 12.5f;
        m_outerCutoff = 25.0f;
        m_range = 10000.0f;


        UpdateViewMat();
        UpdateProjMat();

		return true;
	}
    void SpotLight::UpdateViewMat()
    {
        m_viewMat = DirectX::XMMatrixLookAtLH(
            DirectX::XMLoadFloat3(&m_position),
            DirectX::XMLoadFloat3(&m_direction),
            DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        UpdateLitMat();
    }
    void SpotLight::UpdateProjMat()
    {
        m_perspectiveMat = DirectX::XMMatrixPerspectiveFovLH(
            DirectX::XMConvertToRadians(m_outerCutoff * 2.0f), 1.0f, _S_NEAR, _S_FAR);
        UpdateLitMat();
    }

    void SpotLight::UpdateLitMat()
    {
        m_lightSpaceMat = DirectX::XMMatrixMultiplyTranspose(m_viewMat, m_perspectiveMat);
    }

    void SpotLight::UpdateProperty()
    {   
        m_spotLightProperty.ambient = m_ambient;
        m_spotLightProperty.attenuation = m_attenuation;
        m_spotLightProperty.diffuse = m_diffuse;
        m_spotLightProperty.direction = m_direction;
        m_spotLightProperty.position = m_position;
        m_spotLightProperty.range = m_range;
        m_spotLightProperty.specular = m_specular;
        m_spotLightProperty.cutoff = DirectX::XMScalarCos(DirectX::XMConvertToRadians(m_cutoff));
        m_spotLightProperty.outerCutoff = DirectX::XMScalarCos(DirectX::XMConvertToRadians(m_outerCutoff));
    
    }
	
}