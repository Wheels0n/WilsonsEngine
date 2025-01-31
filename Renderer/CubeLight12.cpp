
#include "CubeLight12.h"
#include "HeapManager.h"
namespace wilson
{
    using namespace std;
    using namespace DirectX;

    constexpr float g_cube_near = 1.0f;
    constexpr float g_cube_far = 1500.0f;
    constexpr float g_cube_ratio = 1.0f;
    constexpr float g_cube_fov = XMConvertToRadians(90.0f);
    XMMATRIX CubeLight12::g_perspective =
        XMMatrixPerspectiveFovLH(g_cube_fov, g_cube_ratio, g_cube_near, g_cube_far);
    vector<XMVECTOR> CubeLight12::g_upVectors = {
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    };
    vector<XMVECTOR> CubeLight12::g_dirVectors = {
        XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f),
        XMVectorSet(-1.0f,0.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f,-1.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
    };

    void CubeLight12::UpdateLightPos()
    {
        XMFLOAT3 pos3 = m_pProperty->position;
        XMVECTOR pos = XMVectorSet(pos3.x, pos3.y, pos3.z, g_cube_far);
        g_pHeapManager->CopyDataToCb(m_cubeLightPosKey, sizeof(XMVECTOR), &pos);
    }
    void CubeLight12::UpdateCubeMatrices()
    {
        XMVECTOR pos = XMLoadFloat3(&m_pProperty->position);
        for (int i = 0; i < _CUBE_FACES; ++i)
        {
            XMMATRIX viewMat =
                XMMatrixLookAtLH(pos, XMVectorAdd(pos, g_dirVectors[i]), g_upVectors[i]);
            m_cubeMatrices[i] = XMMatrixMultiplyTranspose(viewMat, g_perspective);
        }

        g_pHeapManager->CopyDataToCb(m_cubeLightMatricesKey, sizeof(XMMATRIX) * _CUBE_FACES, &m_cubeMatrices[0]);
    }
    void CubeLight12::UploadCubeLightMatrices(ComPtr<ID3D12GraphicsCommandList> pCmdList)
    {
        g_pHeapManager->UploadGraphicsCb(m_cubeLightMatricesKey, E_TO_UINT(eCubeShadowRP::gsMat), pCmdList);
        return;
    }
    void CubeLight12::UploadLightPos(ComPtr<ID3D12GraphicsCommandList> pCmdList)
    {
        g_pHeapManager->UploadGraphicsCb(m_cubeLightPosKey, E_TO_UINT(eCubeShadowRP::psLightPos), pCmdList);
        return;
    }


    CubeLight12::CubeLight12(const UINT idx)
        :Light12(idx)
    {
        UINT cbSize                 = sizeof(XMMATRIX) * (_CUBE_FACES);
        m_cubeLightMatricesKey      = g_pHeapManager->AllocateCb(cbSize);
                
        cbSize                      = sizeof(XMVECTOR);
        m_cubeLightPosKey           =  g_pHeapManager->AllocateCb(cbSize);
 
        m_cubeMatrices.resize(_CUBE_FACES);
        UpdateCubeMatrices();
    }

    CubeLight12::~CubeLight12()
    {
        Light12::~Light12();
    }
}
