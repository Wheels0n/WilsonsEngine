#pragma once

#include "typedef.h"
#include "Object11.h"
#include "DirectionalLight11.h"
#include "CubeLight11.h"
#include "SpotLight11.h"

namespace wilson
{  
	class Entity11
	{
	public:
		inline void DecreaseEntityIndex()
		{
			--m_entityIdx;
		}
		inline void DecreaseLightIndex()
		{
			--m_lightIdx;
		}
		inline void DecreaseObjectIndex()
		{
			--m_objectIdx;
		}
		inline int GetEntityIndex() const
		{
			return m_entityIdx;
		}
		inline Light11* GetLight()
		{
			return m_pLight;
		}
		inline int GetLightIndex() const
		{
			return m_lightIdx;
		}
		inline std::string GetName()
		{
			return m_name;
		}
		inline int GetNumInstance()
		{
			return m_pObject->GetNumInstance();
		}
		inline Object11* GetpObject() const
		{
			return m_pObject;
		}
		inline int GetObjectIndex() const
		{
			return m_objectIdx;
		}
		inline void SetNumInstance(UINT n)
		{
			m_pObject->SetNumInstance(n);
		}
		inline void ToggleInstancing()
		{
			m_pObject->ToggleInstancing();
		}
		inline bool IsObject()
		{
			return m_bObject;
		}

		Entity11(const std::string, const UINT, Object11* const, const UINT);
		Entity11(const std::string, const UINT, Light11* const, const UINT);
		Entity11(const Entity11&) = default;
		~Entity11() = default;
	private:
		bool m_bObject;
		std::string m_name;
		int m_entityIdx;
		int m_lightIdx;
		int m_objectIdx;

		Light11* m_pLight;
		Mesh11* m_pSelectedMesh;
		Object11* m_pObject;
	};
}


