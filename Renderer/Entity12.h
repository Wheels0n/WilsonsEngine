#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <string>

#include "Object.h"
#include "DirectionalLight12.h"
#include "CubeLight12.h"
#include "SpotLight12.h"

namespace wilson
{
	class Entity12
	{
	public:
		//EntityIdx는 현 클새에서 접근 할 용도. 나머지 lightIdx, modelIdx는 d3d12클래스에서 접근할 용도.
		inline bool isObject()
		{
			return m_isObject;
		}
		inline Light12* GetLight()
		{
			return m_pLight;
		}
		inline Object* GetpObject() const
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

		Entity12(std::string, UINT, Object*, UINT);
		Entity12(std::string, UINT, Light12*, UINT);
		Entity12(const Entity12&) = default;
		~Entity12() = default;
	private:
		bool m_isObject;
		Light12* m_pLight;
		Object* m_pObject;
		Mesh* m_pSelectedModel;
		std::string m_Name;
		UINT m_entityIdx;
		UINT m_lightIdx;
		UINT m_objectIdx;
	};
}


