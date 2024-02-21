#pragma once
#include <DirectXMath.h>
#include "typedef.h"
namespace wilson
{
	class Renderer
	{	
	public:
		virtual void BeginFrame() {};
		virtual void EndFrame() {};

		virtual void UpdateResolution(int, int) {};
		virtual void Translate(DirectX::XMVECTOR) {};
		virtual void Rotate(int dx, int dy) {};
		virtual ~Renderer() {};
	protected:
		eAPI m_eAPI;
	};
}