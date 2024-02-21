#pragma once 

namespace wilson {

	class Editor
	{
	public:
		virtual void Draw() {};
		virtual void Pick() {};
		virtual bool CheckRange(int, int) { return false; };
		virtual ~Editor() {};
	};
}
