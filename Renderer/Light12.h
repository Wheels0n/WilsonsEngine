#pragma once

#include <dxgi.h>
#include <d3d12.h>

#include "Camera12.h"
#include "typedef.h"
#define _DEFAULT_CUTOFF			15.0f
#define _DEFAULT_OUTER_CUTOFF	25.0f
#define _MAX_LIGHT_INTENSITY	1000.0f
namespace wilson
{
	using namespace std;
	using namespace DirectX;
	class Light12
	{
		public:
			shared_ptr<LightProperty>	GetProperty();
			UINT						GetLightIndex();
			BOOL						GetDirty();
			
			void						SetDirty(BOOL);

			eLIGHT_TYPE virtual			GetType() { return eLIGHT_TYPE::CUBE; };

										Light12(const UINT);
			virtual						~Light12() = default;
		protected:
			shared_ptr<LightProperty>	m_pProperty;

			BOOL						m_bDirty;
			UINT						m_Idx;
	};
}