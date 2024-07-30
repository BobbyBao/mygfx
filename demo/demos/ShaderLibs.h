#pragma once
#include "Program.h"

namespace mygfx::demo {

	class ShaderLibs {
	public:

		static utils::Ref<Program> getColorShader();
		static utils::Ref<Program> getBasicShader();
		static void clean();
	};

}