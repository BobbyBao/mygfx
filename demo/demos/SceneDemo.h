#include "DemoApp.h"
#include "Framework.h"
#include "CameraController.h"


namespace mygfx::demo {
	
	class SceneDemo : public Demo {
	public:
		Ref<View> mView;
		Ref<Scene> mScene;
		Ref<Node> mCameraNode;
		Ref<Camera> mCamera;
		CameraController mCameraController;

		Result<void> start() override {

			mScene = new Scene();

			float w = (float)mApp->getWidth();
			float h = (float)mApp->getHeight();

			mCameraNode = new Node();
			mCamera = mCameraNode->addComponent<Camera>();
			mCamera->setPerspective(45.0f, w / h, 0.1f, 100.0f);

			mCameraController.init(mCamera);
			mCameraController.lookAt(vec3{ 0.0f, 0.0f, 4.0f }, vec3{ 0.0f });

			mView = new View(mApp->getSwapChain());
			mView->setScene(mScene);
			mView->setCamera(mCamera);

			auto skybox = mScene->createChild<Node>()->addComponent<Skybox>();

			skybox->setCubeMap(Texture::createFromFile("textures/papermill/specular.dds", SamplerInfo{ .srgb = true }));
			skybox->setIrrMap(Texture::createFromFile("textures/papermill/diffuse.dds", SamplerInfo{ .srgb = true }));

			mView->setEnvIntensity(0.75f);

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

}