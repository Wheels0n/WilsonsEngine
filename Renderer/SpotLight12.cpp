#include "SpotLight12.h"
#include "HeapManager.h"

namespace wilson
{

    SpotLight12::SpotLight12(const UINT idx)
        :Light12(idx), m_cutoff(_DEFAULT_CUTOFF), m_outerCutoff(_DEFAULT_OUTER_CUTOFF)
    {
        m_pCompositeMatrices = make_shared<CompositeMatrices>();
        m_pWVPMatrices = make_shared<WVPMatrices>();

        m_spotLightMatrixKey = g_pHeapManager->AllocateCb(sizeof(XMMATRIX));
    }
    SpotLight12::~SpotLight12()
    {
        Light12::~Light12();
    }
    XMMATRIX* SpotLight12::GetSpotLightSpaceMatrix()
    {
        return &m_pCompositeMatrices->vpMat;
    }

    void SpotLight12::UpdateViewMatrix()
    {
        XMVECTOR dir = XMLoadFloat3(&m_pProperty->direction);
        dir = XMVector3Normalize(dir);

        XMVECTOR pos    = XMLoadFloat3(&m_pProperty->position);
        XMVECTOR target = XMVectorAdd(pos, dir);

        m_pWVPMatrices->vMat = XMMatrixLookAtLH(
            pos,
            target,
            XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        UpdateSpotLightMatrix();
    }
    void SpotLight12::UpdateProjMatrix()
    {
        m_pWVPMatrices->pMat = XMMatrixPerspectiveFovLH(
            XMConvertToRadians(m_outerCutoff * 2.0f), 1.0f, _S_NEAR, _S_FAR);
        UpdateSpotLightMatrix();
    }
    void SpotLight12::UpdateSpotLightMatrix()
    {
        m_pCompositeMatrices->vpMat = XMMatrixMultiplyTranspose(m_pWVPMatrices->vMat, m_pWVPMatrices->pMat);
        g_pHeapManager->CopyDataToCb(m_spotLightMatrixKey, sizeof(XMMATRIX), &m_pCompositeMatrices->vpMat);
    }

    void SpotLight12::UploadSpotLightMatrix(ComPtr<ID3D12GraphicsCommandList> pCmdList)
    {
        g_pHeapManager->UploadGraphicsCb(m_spotLightMatrixKey, 1, pCmdList);
    }
}