#include"DirectionalLight.h"

namespace wilson
{
    bool DirectionalLight::Init(ID3D11Device* pDevice)
    {
        D3D11_BUFFER_DESC lightCBD;
        lightCBD.Usage = D3D11_USAGE_DYNAMIC;
        lightCBD.ByteWidth = sizeof(DirLightProperty);
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
        return true;
    }

    void DirectionalLight::UpdateProperty()
    {
        m_dirLightProperty.ambient = m_ambient;
        m_dirLightProperty.diffuse = m_diffuse;
        m_dirLightProperty.specular = m_specular;
        m_dirLightProperty.position = m_position;
    }
}