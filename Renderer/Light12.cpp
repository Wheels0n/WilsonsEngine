#include "Light12.h"

namespace wilson
{
	Light12::Light12(const UINT idx)
		:m_bDirty(false), m_Idx(idx)
	{
		m_pProperty = make_shared<LightProperty>();

		m_pProperty->ambient		= XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		m_pProperty->diffuse		= XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		m_pProperty->specular		= XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

		m_pProperty->position		= XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_pProperty->direction		= XMFLOAT3(0.0f, 0.0f, 0.5f);
		m_pProperty->attenuation	= XMFLOAT3(0.0f, 0.0f, 0.0f);

		m_pProperty->range			= 0.0f;
		m_pProperty->cutoff			= _DEFAULT_CUTOFF;
		m_pProperty->outerCutoff	= _DEFAULT_OUTER_CUTOFF;
	}

	shared_ptr<LightProperty> Light12::GetProperty()
	{
		return m_pProperty;
	}
	UINT Light12::GetLightIndex()
	{
		return m_Idx;
	}
	BOOL Light12::GetDirty()
	{
		return m_bDirty;
	}
	void Light12::SetDirty(BOOL bDirty)
	{
		m_bDirty = bDirty;
	}
}