#pragma once

#include <d3d12.h>

#include "../ImGui/imgui.h"
#include "D3D12.h"
#include "Scene12.h"
#include "Import12.h"
#include "typedef.h"
namespace wilson
{
	using namespace std;
	using namespace DirectX;
	class Viewport12
	{
		public:
			BOOL							CheckRange(const int, const int);
		
			int								GetX();
			int								GetY();
			float							GetNDCX(const int);
			float							GetNDCY(const int);

			void							Draw();
			void							Resize();

											Viewport12(shared_ptr<D3D12>, shared_ptr<Scene12>);
											Viewport12(const Viewport12&) = delete;
											~Viewport12()=default;
		private:
			XMVECTOR						CalEntityPos();
		private:
			BOOL							m_IsFocused;

			UINT							m_width;
			UINT							m_height;

			int								m_left;
			int								m_top;

			shared_ptr<D3D12>				m_pD3D12;
			shared_ptr<Camera12>			m_pCam;
			shared_ptr<Importer12>			m_pImporter;
			shared_ptr<Scene12>				m_pScene;
	};
}

