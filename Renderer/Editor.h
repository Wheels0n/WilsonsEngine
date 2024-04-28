#pragma once 

namespace wilson {

	class Editor
	{
	public:
		virtual bool CheckRange(int, int) { return false; };
		virtual void Draw() {};
		virtual void Pick() {};

		virtual ~Editor() {};
	};
}
