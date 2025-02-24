#pragma once
#include "resource/Shader.h"

namespace mygfx::samples {

	class ShaderLibs {
	public:

		static utils::Ref<Shader> getColorShader();
		static utils::Ref<Shader> getUnlitShader();
		static utils::Ref<Shader> getSimpleLightShader();
		static Shader* getFullscreenShader();
		static void clean();
	};

}