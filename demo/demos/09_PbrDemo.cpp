#include "SceneDemo.h"

namespace mygfx::demo {

class PbrDemo : public SceneDemo {
public:
    Result<void> start() override
    {
        SceneDemo::start();

        ModelLoader modelLoader;
        auto model = modelLoader.load("models/BoomBox/glTF/BoomBox.gltf");
        float boundSize = length(modelLoader.getBoundingBox().size());
        if (boundSize < 1.0f) {
            model->setScale(vec3 { 1.0f / boundSize });
        }

        mScene->addChild(model);
        co_return;
    }

    void gui() override
    {
        auto& frameUniforms = mView->frameUniforms();

        ImGui::SliderFloat("envIntensity", &frameUniforms.envIntensity, 0.0f, 10.0f);
        ImGui::SliderFloat("exposure", &frameUniforms.exposure, 0.0f, 10.0f);
    }
};

DEF_DEMO(PbrDemo, "Pbr Demo");
}