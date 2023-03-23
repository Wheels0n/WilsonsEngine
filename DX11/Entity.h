#ifndef ENTITY_H
#define ENTITY_H

#include <Windows.h>
#include <DirectXMath.h>
#include <string>
#include "Model.h"

namespace wilson
{
	class Entity
	{
	public:
		inline std::string* GetType() 
		{
			return &m_type;
		};
		inline Model* GetModel() const
		{
			return m_pModel;
		}

		Entity(std::string, Model*);
		Entity(const Entity&) = default;
		~Entity() = default;
	private:
		Model* m_pModel;
		std::string m_type;
	};
}
#endif // !ENTITY_H

