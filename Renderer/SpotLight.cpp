#include "SpotLight.h"

namespace wilson
{   

	SpotLight::SpotLight(ID3D11Device* pDevice, UINT idx)
        :Light(idx)
	{	
        D3D11_BUFFER_DESC cbufferDesc;
        cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbufferDesc.ByteWidth = sizeof(SpotLightProperty);
        cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbufferDesc.MiscFlags = 0;
        cbufferDesc.StructureByteStride = 0;

        HRESULT result = pDevice->CreateBuffer(&cbufferDesc, nullptr, &m_pLightBuffer);
        if (FAILED(result))
        {
            OutputDebugStringA("SpotLight::m_pLightBuffer::CreateBufferFailed");
        }
        m_pLightBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            sizeof("SpotLight::m_pLightBuffer") - 1, "SpotLight::m_pLightBuffer");

        m_direction =   DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
        m_attenuation = DirectX::XMFLOAT3(0.1f, 0.01f, 0.001f);
        m_cutoff = 12.5f;
        m_outerCutoff = 25.0f;
        m_range = 10000.0f;


        UpdateViewMat();
        UpdateProjMat();

	}
    void SpotLight::UpdateViewMat()
    {
        DirectX::XMVECTOR dir = XMLoadFloat3(&m_direction);
        dir = DirectX::XMVector3Normalize(dir);

        DirectX::XMVECTOR pos = XMLoadFloat3(&m_position);
        DirectX::XMVECTOR target = DirectX::XMVectorAdd(pos, dir);
        DirectX::XMFLOAT3 target3;
        DirectX::XMStoreFloat3(&target3, target);

        m_viewMat = DirectX::XMMatrixLookAtLH(
            DirectX::XMLoadFloat3(&m_position),
            DirectX::XMLoadFloat3(&target3),
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