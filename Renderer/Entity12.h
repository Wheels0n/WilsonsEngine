#pragma once
#include "typedef.h"
#include "Object12.h"
#include "DirectionalLight12.h"
#include "CubeLight12.h"
#include "SpotLight12.h"

namespace wilson
{
	class Entity12
	{
	public:
		//EntityIdx는 현 클래스에서 접근 할 용도. 
		//나머지 lightIdx, objectlIdx는 d3d12클래스에서 접근할 용도.
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
		inline Light12* GetLight()
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
		inline Object12* GetpObject() const
		{
			return m_pObject;
		}
		inline int GetObjectIndex() const
		{
			return m_objectIdx;
		}
		inline bool IsObject()
		{
			return m_bObject;
		}

		Entity12(const std::string, const UINT, Object12* const, const UINT);
		Entity12(const std::string, const UINT, Light12* const, const UINT);
		Entity12(const Entity12&) = default;
		~Entity12() = default;
	private:
		bool m_bObject;
		std::string m_name;
		
		int m_entityIdx;
		int m_lightIdx;
		int m_objectIdx;

		Light12* m_pLight;
		Object12* m_pObject;
		Mesh12* m_pSelectedMesh;
	};
}


