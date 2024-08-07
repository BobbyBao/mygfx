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
			//mCameraController.setMode(CameraController::Mode::FreeFlight);
			mCameraController.lookAt(vec3{ 0.0f, 0.0f, 4.0f }, vec3{ 0.0f });

			mView = new View(mApp->getSwapChain());
			mView->setScene(mScene);
			mView->setCamera(mCamera);

			//mScene->addChild(createCube());

			auto skybox = mScene->createChild<Skybox>();
			skybox->setCubeMap(Texture::createFromFile("textures/papermill/specular.dds", SamplerInfo {.srgb = true }));
			auto irr = Texture::createFromFile("textures/papermill/diffuse.dds", SamplerInfo{ .srgb = true });
			skybox->setIrrMap(irr);

			ModelLoader modelLoader;
			auto model = modelLoader.load("models/BoomBox/glTF/BoomBox.gltf");
			//auto model = modelLoader.load("models/Suzanne/glTF/Suzanne.gltf");
			float boundSize = length(modelLoader.getBoundingBox().size());
			if (boundSize < 1.0f) {
				model->setScale(vec3{ 1.0f / boundSize });
			}
			
			mView->setEnvIntensity(0.75f);

			mScene->addChild(model);
			co_return;
		}

		Ref<Renderable> createCube()
		{
			Ref<Mesh> mesh(Mesh::createCube());
			DefineList macros;
			macros.add("HAS_TEXCOORD_0_VEC2", 1)
				.add("HAS_NORMAL_VEC3", 2)
				.add("MATERIAL_METALLICROUGHNESS")
				.add("LINEAR_OUTPUT")
				.add("USE_IBL")
				.add("DEBUG_NONE", 0)
				.add("DEBUG", 0);

			mShader = Shader::fromFile("shaders/primitive.vert", "shaders/pbr.frag", &macros);
			mShader->setVertexInput({ Format::R32G32B32_SFLOAT, Format::END, Format::R32G32_SFLOAT, Format::END, Format::R32G32B32_SFLOAT });

			auto material = new Material(mShader, "MaterialUniforms");
			mesh->setMaterial(material);
			material->setShaderParameter("u_BaseColorFactor", vec4{ 1.0f, 0.0f, 0.0f, 1.0f });
			material->setShaderParameter("u_MetallicFactor", 0.0f);
			material->setShaderParameter("u_RoughnessFactor", 0.5f);

			Ref<Renderable> node(new Renderable());
			node->setMesh(mesh);
			node->setPosition({ 0.0f, 0.0f, 0.0f });
			return node;
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