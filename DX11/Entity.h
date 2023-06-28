#ifndef ENTITY_H
#define ENTITY_H

#include <Windows.h>
#include <DirectXMath.h>
#include <string>
#include "ModelGroup.h"

namespace wilson
{
	class Entity
	{
	public:
		inline ModelGroup* GetModelGroup() const
		{
			return m_pModelGroup;
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
		Entity(const Entity&) = default;
		~Entity() = default;
	private:
		ModelGroup* m_pModelGroup;
		std::string m_Name;
	};
}
#endif // !ENTITY_H

