#include "DemoApp.h"
#include "Framework.h"
#include "CameraController.h"

namespace mygfx::demo {
	
	class PbrDemo : public Demo {
	public:
		Ref<View> mView;
		Ref<Scene> mScene;
		Ref<Camera> mCamera;
		Ref<Mesh> mMesh;
		Ref<Shader> mShader;
		CameraController mCameraController;
		Result<void> start() override {
			
			mScene = new Scene();
			
			float w = (float)mApp->getWidth();
			float h = (float)mApp->getHeight();
			mCamera = new Camera();
			mCamera->setPerspective(45.0f, w / h, 0.1f, 100.0f);
			mCameraController.init(mCamera);
			//mCameraController.setMode(CameraController::Mode::FreeFlight);
			mCameraController.lookAt(vec3{ 0.0f, 0.0f, 4.0f }, vec3{ 0.0f });

			mView = new View(mApp->getSwapChain());
			mView->setScene(mScene);
			mView->setCamera(mCamera);

			mMesh = Mesh::createCube();
			
			
			DefineList macros;
			macros.add("HAS_TEXCOORD_0_VEC2", 1);
			macros.add("HAS_NORMAL_VEC3", 2);
			mShader = Shader::fromFile("shaders/primitive.vert", "shaders/pbr.frag", &macros);
			mShader->setVertexInput({ Format::R32G32B32_SFLOAT, Format::END, Format::R32G32_SFLOAT, Format::END, Format::R32G32B32_SFLOAT});

			auto material = new Material(mShader, "MaterialUniforms");
			mMesh->setMaterial(material);
			material->setShaderParameter("u_BaseColorFactor", vec4{1.0f});
			material->setShaderParameter("u_MetallicFactor", vec4{0.0f});
			material->setShaderParameter("u_RoughnessFactor", vec4{0.5f});
			material->setShaderParameter("u_BaseColorTexture", Texture::Green);

			auto node = new Renderable();
			node->setMesh(mMesh);
			node->setPosition({0.0f, 0.0f, 0.0f});
			mScene->addChild(node);

			//ModelLoader modelLoader;
			//auto model = modelLoader.load("models/cube.gltf");
			
			//mScene->addChild(model);

			auto skybox = mScene->createChild<Skybox>();
			skybox->setCubeMap(Texture::createFromFile("textures/hdr/papermill.ktx"));

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