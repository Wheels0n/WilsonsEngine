#pragma once

#include "typedef.h"
namespace wilson
{
	class Renderer
	{	
	public:
		virtual void BeginFrame() {};
		virtual void EndFrame() {};

		virtual void Rotate(int dx, int dy) {};
		virtual void Translate(DirectX::XMVECTOR) {};
		virtual void UpdateResolution(const UINT, const UINT) {};

		virtual ~Renderer() {};
	protected:
		eAPI m_eAPI;
	};
}