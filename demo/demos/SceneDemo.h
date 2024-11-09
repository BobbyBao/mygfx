#include "CameraController.h"
#include "DemoApp.h"
#include "Framework.h"

namespace mygfx::demo {

class SceneDemo : public Demo {
public:
    Ref<View> mView;
    Ref<Scene> mScene;
    Ref<Node> mCameraNode;
    Ref<Camera> mCamera;
    CameraController* mCameraController = nullptr;

    void start() override
    {
        mScene = new Scene();

        float w = (float)mApp->getWidth();
        float h = (float)mApp->getHeight();

        mCameraNode = new Node();
        mCamera = mCameraNode->addComponent<Camera>();
        mCamera->setPerspective(45.0f, w / h, 0.1f, 100.0f);

        mCameraController = mCameraNode->addComponent<CameraController>();
        mCameraController->lookAt(vec3 { 0.0f, 0.0f, 4.0f }, vec3 { 0.0f });

        mView = mApp->createView(mApp->getSwapChain());
        mView->setScene(mScene);
        mView->setCamera(mCamera);

        auto skybox = mScene->createChild<Node>()->addComponent<Skybox>();

        SamplerInfo sampler = SamplerInfo::create(Filter::LINEAR, SamplerAddressMode::CLAMP_TO_EDGE, true);
        skybox->setCubeMap(Texture::createFromFile("textures/papermill/specular.dds", sampler));
        skybox->setIrrMap(Texture::createFromFile("textures/papermill/diffuse.dds", sampler));

    }
    
    void stop() override 
    {
        mApp->destroyView(mView);
    }

    void update(double delta) override
    {
        mCameraController->update((float)delta);
    }
    
    Ref<Node> loadModel(const String& filePath)
    {
        ModelLoader modelLoader;
        auto model = modelLoader.load(filePath);
        float boundSize = length(modelLoader.getBoundingBox().size());
        if (boundSize < 1.0f) {
            model->setScale(vec3 { 1.0f / boundSize });
        }

        return mScene->instantiate(model);

    }
};

}