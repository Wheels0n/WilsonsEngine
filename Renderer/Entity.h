#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <string>

#include "ModelGroup.h"
#include "DirectionalLight.h"
#include "CubeLight.h"
#include "SpotLight.h"

namespace wilson
{  
	class Entity
	{
	public:
		inline bool isObject()
		{
			return m_isObject;
		}
		inline Light* GetLight()
		{
			return m_pLight;
		}
		inline ModelGroup* GetpObject() const
		{
			return m_pObject;
		}
		inline std::string GetName()
		{
			return m_Name;
		}
		inline UINT GetEntityIndex() const
		{
			return m_entityIdx;
		}
		inline UINT GetLightIndex() const
		{
			return m_lightIdx;
		}
		inline UINT GetObjectIndex() const
		{
			return m_objectIdx;
		}
		inline void DecreaseLightIndex()
		{
			--m_lightIdx;
		}
		inline void DecreaseObjectIndex()
		{
			--m_objectIdx;
		}
		inline void DecreaseEntityIndex()
		{
			--m_entityIdx;
		}
		inline void ToggleInstancing()
		{
			m_pObject->ToggleInstancing();
		}
		inline int GetNumInstance()
		{
			return m_pObject->GetNumInstance();
		}
		inline void SetNumInstance(int n)
		{
			m_pObject->SetNumInstance(n);
		}

		Entity(std::string, UINT idx, ModelGroup*, UINT modelIdx);
		Entity(std::string, UINT idx, Light*,  UINT lightIdx);
		Entity(const Entity&) = default;
		~Entity() = default;
	private:
		bool m_isObject;
		Light* m_pLight;
		ModelGroup* m_pObject;
		Model* m_pSelectedModel;
		std::string m_Name;
		UINT m_entityIdx;
		UINT m_lightIdx;
		UINT m_objectIdx;
	};
}


