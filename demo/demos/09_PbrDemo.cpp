#include "SceneDemo.h"

namespace mygfx::demo {
	
	class PbrDemo : public SceneDemo {
	public:
		Ref<Shader> mShader;
		Result<void> start() override {
			
			SceneDemo::start();

			ModelLoader modelLoader;
			auto model = modelLoader.load("models/BoomBox/glTF/BoomBox.gltf");
			float boundSize = length(modelLoader.getBoundingBox().size());
			if (boundSize < 1.0f) {
				model->setScale(vec3{ 1.0f / boundSize });
			}
			
			mScene->addChild(model);
			co_return;
		}

	};

	DEF_DEMO(PbrDemo, "Pbr Demo");	
}