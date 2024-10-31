#include "SceneDemo.h"

namespace mygfx::demo {

class InstancingDemo : public SceneDemo {
public:
    void start() override
    {
        SceneDemo::start();

        ModelLoader modelLoader;
        auto model = modelLoader.load("models/teapots_galore/teapots_galore.gltf");
        mScene->addChild(model);
    }

    void gui() override
    {
    }
};

DEF_DEMO(InstancingDemo, "Instancing Demo");
}