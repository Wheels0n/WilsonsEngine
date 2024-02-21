#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <string>

#include "ModelGroup12.h"
#include "DirectionalLight12.h"
#include "PointLight12.h"
#include "SpotLight12.h"

namespace wilson
{
	class Entity12
	{
	public:
		//EntityIdx는 현 클새에서 접근 할 용도. 나머지 lightIdx, modelIdx는 d3d12클래스에서 접근할 용도.
		inline bool isModel()
		{
			return m_isModel;
		}
		inline Light12* GetLight()
		{
			return m_pLight;
		}
		inline ModelGroup12* GetModelGroup() const
		{
			return m_pModelGroup12;
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
			m_pModelGroup12->ToggleInstancing();
		}
		inline int GetNumInstance()
		{
			return m_pModelGroup12->GetNumInstance();
		}
		inline void SetNumInstance(int n)
		{
			m_pModelGroup12->SetNumInstance(n);
		}

		Entity12(std::string, UINT, ModelGroup12*, UINT);
		Entity12(std::string, UINT, Light12*, UINT);
		Entity12(const Entity12&) = default;
		~Entity12() = default;
	private:
		bool m_isModel;
		Light12* m_pLight;
		ModelGroup12* m_pModelGroup12;
		Model12* m_pSelectedModel;
		std::string m_Name;
		UINT m_entityIdx;
		UINT m_lightIdx;
		UINT m_modelGroupIdx;
	};
}


