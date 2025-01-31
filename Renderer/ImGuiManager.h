#pragma once
#include "typedef.h"
namespace wilson {
	using namespace DirectX;
	class ImGuiManager
	{
	public:
		static BOOL DrawFloat4(XMFLOAT4& pValue, const char* pText, const float speed = 0.01f);
		static BOOL DrawFloat3(XMFLOAT3& pValue, const char* pText, const float speed = 0.01f);
		static BOOL DrawVec3Control(const std::string& label, float* vals);

		void UpdateMatrices();

		ImGuiManager();
		~ImGuiManager();
	private:
		void SetDockspace();
		void SetMenuBar();
	};
}
