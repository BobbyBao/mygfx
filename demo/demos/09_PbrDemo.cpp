#include "DemoApp.h"
#include "resource/Mesh.h"
#include "resource/Texture.h"
#include "resource/Material.h"
#include "resource/ModelLoader.h"
#include "scene/Scene.h"
#include "scene/Renderable.h"
#include "scene/Camera.h"

namespace mygfx::demo {
	
	class PbrDemo : public Demo {
	public:
		Ref<Scene> mScene;
		Ref<Camera> mCamera;
		Ref<Mesh> mMesh;

		Result<void> start() override {
			
			mScene = new Scene();

			auto shader = ShaderLibs::getSimpleLightShader();

			mMesh = Mesh::createCube();
			
			auto material = new Material(shader, "MaterialUniforms");
			mMesh->setMaterial(material);
			material->setShaderParameter("baseColor", Texture::Green);

			auto node = new Renderable();
			node->setMesh(mMesh);

			mScene->addChild(node);

			mCamera = new Camera();
			mCamera->lookAt(vec3{0.0f, 0.0f, 4.0f}, vec3{0.0f}, vec3{0.0f, 1.0f, 0.0f});

			co_return;
		}

		void draw(GraphicsApi& cmd) override {

			float w = (float)mApp->getWidth();
			float h = (float)mApp->getHeight();

			mCamera->setPerspective(45.0f, w / h, 0.1f, 100.0f);
			auto vp = mCamera->getProjMatrix() * mCamera->getViewMatrix();
			uint32_t perView = gfxApi().allocConstant(vp);

			for (auto renderable : mScene->renderables) {

				auto& world = renderable->getWorldTransform();
				uint32_t perDraw = gfxApi().allocConstant(world);

				for (auto& prim : renderable->primitives) {
					
					uint32_t perMaterial = prim.material->getMaterialUniforms();

					cmd.bindPipelineState(prim.material->getPipelineState());
					cmd.bindUniforms({ perView, perDraw, perMaterial });
					cmd.drawPrimitive(prim.renderPrimitive);
				}
			}

		}
	};

	DEF_DEMO(PbrDemo, "Pbr Demo");	
}