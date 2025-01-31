#pragma once
#include "Light12.h"
#include "typedef.h"
namespace wilson
{
	using namespace std;
	using namespace DirectX;
	using namespace Microsoft::WRL;
	class HeapManager;
	class CubeLight12 :public Light12
	{
		public:
			eLIGHT_TYPE				GetType() { return eLIGHT_TYPE::CUBE; };

			void					UpdateLightPos();
			void					UpdateCubeMatrices();

			void					UploadLightPos(ComPtr<ID3D12GraphicsCommandList>);
			void					UploadCubeLightMatrices(ComPtr<ID3D12GraphicsCommandList>);

									CubeLight12(const UINT idx);
									CubeLight12(const CubeLight12&)=default;
									~CubeLight12();

		private:
			UINT					m_cubeLightMatricesKey;
			UINT					m_cubeLightPosKey;

			vector<XMMATRIX>		m_cubeMatrices;

			static XMMATRIX			g_perspective;
			static vector<XMVECTOR> g_dirVectors;
			static vector<XMVECTOR> g_upVectors;

	};
}