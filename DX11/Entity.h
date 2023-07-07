#ifndef ENTITY_H
#define ENTITY_H

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
		Entity(std::string, ModelGroup*);
		Entity(std::string, Light*);
		Entity(const Entity&) = default;
		~Entity() = default;
	private:
		bool m_isModel;
		Light* m_pLight;
		ModelGroup* m_pModelGroup;
		Model* m_pSelectedModel;
		std::string m_Name;
	};
}
#endif // !ENTITY_H

