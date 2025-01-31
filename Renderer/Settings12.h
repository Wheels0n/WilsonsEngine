#pragma once 

#include"D3D12.h"
#include"FPS.h"
#include"Camera12.h"
#include"Light12.h"

namespace wilson
{	
	using namespace Microsoft::WRL;
	using namespace std;
	using namespace DirectX;
	
	class HeapManager;
	class Settings12
	{
		public:
			void								Draw();

												Settings12(shared_ptr<D3D12>pD3D12);
												Settings12(const Settings12&) = delete;
												~Settings12() = default;
		private:
			//D3D12 

			shared_ptr<TextureWrapper>				m_pCubeLitIconTex;
			shared_ptr<TextureWrapper>				m_pDirLitIconTex;
			shared_ptr<TextureWrapper>				m_pSpotLitIconTex;


			shared_ptr<Camera12>				m_pCam;
			shared_ptr<D3D12>					m_pD3D12;
			shared_ptr<Light12>					m_pLight;

			FPS									m_FPS;
	};
}

