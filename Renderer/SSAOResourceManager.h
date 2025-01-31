#pragma once
#include <d3d12.h>
#include "typedef.h"
namespace wilson
{
	struct TextureWrapper;
	class SSAOResourceManager
	{
	private:
		void	UploadSSAO(bool bUAV, ComPtr<ID3D12GraphicsCommandList> pCmdList);
	public:

			inline UINT* GetNumSsaoSample()
			{
				return &m_nSsaoSample;
			}

			inline float* GetSsaoBias()
			{
				return &m_ssaoBias;
			}
			inline float* GetSsaoRadius()
			{
				return &m_ssaoRadius;
			}
	
			void	DrawSSAOBlurredImage();

			void	Init(UINT width, UINT height);

			void	UpdateSSAOParemeters();
			void	UpdateProjMat(XMMATRIX*);
		
			void	SetLightingPass(ComPtr<ID3D12GraphicsCommandList> pCmdList);
			void	SetSSAOPass(ComPtr<ID3D12GraphicsCommandList> pCmdList);
			void	SetSSAOBlurPass(ComPtr<ID3D12GraphicsCommandList> pCmdList);

			void	SetBeginSceneBarriers();
			void	SetSAAOBlurPassBarriers();
			void	SetLightPassBarriers();
			SSAOResourceManager(UINT, UINT, ComPtr<ID3D12GraphicsCommandList>);
			~SSAOResourceManager();

		private:
			UINT m_ssaoKernalCbKey;
			UINT m_ssaoParametersCbKey;
			UINT m_matrixCbKey;

			UINT m_nSsaoSample;
			float m_ssaoBias;
			float m_ssaoRadius;

			shared_ptr<WVPMatrices>			m_wvpMatrices;

			shared_ptr<TextureWrapper>		m_pSsaoTex;
			shared_ptr<TextureWrapper>		m_pSsaoBlurTex;
			shared_ptr<TextureWrapper>		m_pSsaoBlurDebugTex;
			shared_ptr<TextureWrapper>		m_pNoiseTex;
	};
}


