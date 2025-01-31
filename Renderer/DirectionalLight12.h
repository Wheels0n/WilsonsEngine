#pragma once
#include<d3d12.h>
#include"Light12.h"
#include"typedef.h"
namespace wilson
{
	using namespace std;
	using namespace DirectX;
	using namespace Microsoft::WRL;
	class HeapManager;
	class Camera12;
	class DirectionalLight12 : public Light12
	{
	public:
		vector<XMMATRIX>&		GetDirLightMatrix();
		eLIGHT_TYPE				GetType() { return eLIGHT_TYPE::DIR; };

		XMMATRIX				UpdateDirLightMatrix(const FLOAT nearZ, const FLOAT farZ);
		void					UpdateDirLightMatrices();

		void					UploadDirLightMatrices(ComPtr<ID3D12GraphicsCommandList>);

								DirectionalLight12(const UINT, shared_ptr<Camera12>);
								DirectionalLight12(const DirectionalLight12&) = default;
								~DirectionalLight12();
	private:
		vector<XMVECTOR>		GetFrustumCornersWorldSpace(const XMMATRIX& viewMat, const XMMATRIX& projMat);
	private:
		shared_ptr<Camera12>	m_pCam;
		vector<XMMATRIX>		m_dirLightMatrices;

		UINT					m_matricesKey;
	};
}