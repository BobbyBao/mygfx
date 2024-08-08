#include "DemoApp.h"
#include "Framework.h"
#include "CameraController.h"

namespace mygfx::demo {
	
	class PbrDemo : public Demo {
	public:
		Ref<View> mView;
		Ref<Scene> mScene;
		Ref<Camera> mCamera;
		Ref<Shader> mShader;
		CameraController mCameraController;
		Result<void> start() override {
			
			mScene = new Scene();
			
			float w = (float)mApp->getWidth();
			float h = (float)mApp->getHeight();
			mCamera = new Camera();
			mCamera->setPerspective(45.0f, w / h, 0.1f, 100.0f);
			mCameraController.init(mCamera);
			mCameraController.lookAt(vec3{ 0.0f, 0.0f, 4.0f }, vec3{ 0.0f });

			mView = new View(mApp->getSwapChain());
			mView->setScene(mScene);
			mView->setCamera(mCamera);

			auto skybox = mScene->createChild<Skybox>();

			skybox->setCubeMap(Texture::createFromFile("textures/papermill/specular.dds", SamplerInfo {.srgb = true }));
			skybox->setIrrMap(Texture::createFromFile("textures/papermill/diffuse.dds", SamplerInfo{ .srgb = true }));
		
			ModelLoader modelLoader;
			auto model = modelLoader.load("models/BoomBox/glTF/BoomBox.gltf");
			float boundSize = length(modelLoader.getBoundingBox().size());
			if (boundSize < 1.0f) {
				model->setScale(vec3{ 1.0f / boundSize });
			}
			
			mView->setEnvIntensity(0.75f);

			mScene->addChild(model);
			co_return;
		}

		void update(double delta) override {
			
			mCameraController.update((float)delta);

			mView->update(delta);
		}

		void draw(GraphicsApi& cmd) override {

			mView->render(cmd);

		}
	};

	DEF_DEMO(PbrDemo, "Pbr Demo");	
}