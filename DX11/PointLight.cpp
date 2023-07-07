#include "PointLight.h"
namespace wilson
{
    bool PointLight::Init(ID3D11Device* pDevice)
    {   
        D3D11_BUFFER_DESC lightCBD;
        lightCBD.Usage = D3D11_USAGE_DYNAMIC;
        lightCBD.ByteWidth = sizeof(PointLightProperty);
        lightCBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        lightCBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        lightCBD.MiscFlags = 0;
        lightCBD.StructureByteStride = 0;

        HRESULT result = pDevice->CreateBuffer(&lightCBD, nullptr, &m_pLightBuffer);
        if (FAILED(result))
        {
            return false;
        }

        Light::Init(pDevice);
        m_attenuation = DirectX::XMFLOAT3(0.0f, 0.1f, 0.0f);
        m_range = 25.0f;
        return true;
    }

    void PointLight::UpdateProperty()
    {
        m_pointLightProperty.ambient = m_ambient;
        m_pointLightProperty.attenuation = m_attenuation;
        m_pointLightProperty.diffuse = m_diffuse;
        m_pointLightProperty.position = m_position;
        m_pointLightProperty.range = m_range;
        m_pointLightProperty.specular = m_specular;
    }

}
