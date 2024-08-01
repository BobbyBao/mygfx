#pragma once
#include "Shader.h"

namespace mygfx::demo {

	class ShaderLibs {
	public:

		static utils::Ref<Shader> getColorShader();
		static utils::Ref<Shader> getUnlitShader();
		static utils::Ref<Shader> getSimpleLightShader();
		static void clean();
	};

}