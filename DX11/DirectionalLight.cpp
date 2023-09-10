#include"DirectionalLight.h"

namespace wilson
{
    bool DirectionalLight::Init(ID3D11Device* pDevice)
    {
        D3D11_BUFFER_DESC lightCBD = {};
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
        m_pLightBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("DirectionalLight::m_pLightBuffer") - 1, "DirectionalLight::m_pLightBuffer");

        m_projMat = DirectX::XMMatrixOrthographicLH(100.0f, 100.0f, 0.1f, 100.0f);
        UpdateViewMat();

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

    void DirectionalLight::UpdateViewMat()
    {
        DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        DirectX::XMVECTOR target = DirectX::XMVectorZero();
        DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&m_position);
        m_viewMat = DirectX::XMMatrixLookAtLH(pos,
            target, up);
        m_lightSpaceMat = DirectX::XMMatrixMultiplyTranspose(m_viewMat, m_projMat);//전치행렬 속성
    }
    void DirectionalLight::UpdateProjMat()
    {
        m_projMat = DirectX::XMMatrixOrthographicLH(100.0f, 100.0f, 0.1f, 100.0f);
    }

}