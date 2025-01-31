
#include"DirectionalLight12.h"
#include"HeapManager.h"
namespace wilson
{
 
    DirectionalLight12::DirectionalLight12(const UINT idx, shared_ptr<Camera12> pCam)
        :Light12(idx), m_pCam(pCam)
    {
        m_dirLightMatrices.resize(_CASCADE_LEVELS);
        m_matricesKey = g_pHeapManager->AllocateCb(sizeof(XMMATRIX) * _CASCADE_LEVELS);
        UpdateDirLightMatrices();

    }
    DirectionalLight12::~DirectionalLight12()
    {
        Light12::~Light12();
    }

    vector<XMMATRIX>& DirectionalLight12::GetDirLightMatrix()
    {
        return m_dirLightMatrices;
    }

    void DirectionalLight12::UploadDirLightMatrices(ComPtr<ID3D12GraphicsCommandList> pCmdList)
    {
        g_pHeapManager->UploadGraphicsCb(m_matricesKey, E_TO_UINT(eCascadeShadowRP::gsMat), pCmdList);
        return;
    }
    void DirectionalLight12::UpdateDirLightMatrices()
    {
        const float nearZ = *m_pCam->GetNearZ();
        const float farZ = *m_pCam->GetFarZ();
        vector<float>& shadowCacadeLevels = m_pCam->GetCascadeLevels();
        for (UINT i = 0; i < shadowCacadeLevels.size(); ++i)
        {
            if (i == 0)
            {
                m_dirLightMatrices[i] = UpdateDirLightMatrix(nearZ, shadowCacadeLevels[i]);
            }
            else if (i < shadowCacadeLevels.size() - 1)
            {
                m_dirLightMatrices[i] = UpdateDirLightMatrix(shadowCacadeLevels[i - 1], shadowCacadeLevels[i]);
            }
            else
            {
                m_dirLightMatrices[i] = UpdateDirLightMatrix(shadowCacadeLevels[i - 1], farZ);
            }
        }
        g_pHeapManager->CopyDataToCb(m_matricesKey, sizeof(XMMATRIX) * _CASCADE_LEVELS, &m_dirLightMatrices[0]);
    }

    vector<XMVECTOR> DirectionalLight12::GetFrustumCornersWorldSpace(const XMMATRIX& viewMat, const XMMATRIX& projMat)
    {
        XMMATRIX viewProMat = XMMatrixMultiply(viewMat, projMat);
        XMMATRIX invVPMat = XMMatrixInverse(nullptr, viewProMat);
        vector<XMVECTOR> corners;
        corners.reserve(8);
        for (UINT x = 0; x < 2; ++x)
        {
            for (UINT y = 0; y < 2; ++y)
            {
                for (UINT z = 0; z < 2; ++z)
                {
                    XMVECTOR corner = XMVectorSet(2.0f * x - 1.0f, 2.0f * y - 1.0f, z, 1.0f);
                    corner = XMVector4Transform(corner, invVPMat);
                    corner = XMVectorScale(corner, 1 / corner.m128_f32[3]);
                    corners.push_back(corner);
                }
            }
        }
        return corners;
    }

    XMMATRIX DirectionalLight12::UpdateDirLightMatrix(const FLOAT nearZ, const FLOAT farZ)
    {
        const float fovY = *(m_pCam->GetFovY());
        const float ratio = m_pCam->GetAspect();

        //Gen subFrustum
        XMMATRIX proj = XMMatrixPerspectiveFovLH(fovY, ratio, nearZ, farZ);
        XMMATRIX view = *(m_pCam->GetViewMatrix());

        const vector<XMVECTOR> corners = GetFrustumCornersWorldSpace(view, proj);

        XMVECTOR center = XMVectorZero();
        for (UINT i = 0; i < corners.size(); ++i)
        {
            center = XMVectorAdd(center, corners[i]);
        }
        center = XMVectorScale(center, (float)1 / corners.size());
        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMVECTOR pos = XMLoadFloat3(&m_pProperty->direction);
        pos = XMVector4Normalize(pos);
        pos = XMVectorSubtract(center, pos);

        const XMMATRIX lightView = XMMatrixLookAtLH(pos, center, up);


        XMVECTOR minV = XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
        XMVECTOR maxV = XMVectorSet(FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN);


        for (UINT i = 0; i < corners.size(); ++i)
        {
            const XMVECTOR vec = XMVector4Transform(corners[i], lightView);
            minV = XMVectorMin(vec, minV);
            maxV = XMVectorMax(vec, maxV);
        }
        XMFLOAT4 min4;
        XMFLOAT4 max4;

        XMStoreFloat4(&min4, minV);
        XMStoreFloat4(&max4, maxV);


        const XMMATRIX lightProj =
            XMMatrixOrthographicOffCenterLH(min4.x, max4.x, min4.y, max4.y, min4.z, max4.z);

        return XMMatrixMultiplyTranspose(lightView, lightProj);
    }


}