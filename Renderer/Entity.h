#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <string>

#include "ModelGroup.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

namespace wilson
{  
	class Entity
	{
	public:
		inline bool isModel()
		{
			return m_isModel;
		}
		inline Light* GetLight()
		{
			return m_pLight;
		}
		inline ModelGroup* GetModelGroup() const
		{
			return m_pModelGroup;
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
		inline UINT GetModelIndex() const
		{
			return m_modelGroupIdx;
		}
		inline void DecreaseLightIndex()
		{
			--m_lightIdx;
		}
		inline void DecreaseModelIndex()
		{
			--m_modelGroupIdx;
		}
		inline void DecreaseEntityIndex()
		{
			--m_entityIdx;
		}
		inline void ToggleInstancing()
		{
			m_pModelGroup->ToggleInstancing();
		}
		inline int GetNumInstance()
		{
			return m_pModelGroup->GetNumInstance();
		}
		inline void SetNumInstance(int n)
		{
			m_pModelGroup->SetNumInstance(n);
		}

		Entity(std::string, UINT idx, ModelGroup*, UINT modelIdx);
		Entity(std::string, UINT idx, Light*,  UINT lightIdx);
		Entity(const Entity&) = default;
		~Entity() = default;
	private:
		bool m_isModel;
		Light* m_pLight;
		ModelGroup* m_pModelGroup;
		Model* m_pSelectedModel;
		std::string m_Name;
		UINT m_entityIdx;
		UINT m_lightIdx;
		UINT m_modelGroupIdx;
	};
}

