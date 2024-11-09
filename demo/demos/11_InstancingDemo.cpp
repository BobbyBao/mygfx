#include "SceneDemo.h"

namespace mygfx::demo {

class InstancingDemo : public SceneDemo {
public:
    Ref<Node> mModel;
    void start() override
    {
        SceneDemo::start();

        mModel = loadModel("models/teapots_galore/teapots_galore.gltf");
        
        mCameraController->lookAt(vec3 { 0.0f, 20.0f, 80.0f }, vec3 { 0.0f });
    }

    void gui() override
    {
    }
};

DEF_DEMO(InstancingDemo, "Instancing Demo");
}