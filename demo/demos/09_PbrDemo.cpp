#include "SceneDemo.h"

namespace mygfx::demo {

class PbrDemo : public SceneDemo {
public:
    Ref<Node> mModel;
    uint32_t mModelIndex = 0;

    inline static const char* sModelFiles[] = {
        "models/Suzanne/glTF/Suzanne.gltf",
        "models/BoomBox/glTF/BoomBox.gltf",
    };

    void start() override
    {
        SceneDemo::start();

        loadModelByIndex(0);
    }

    void loadModelByIndex(uint32_t index)
    {
        if (index >= std::size(sModelFiles)) {
            return;
        }

        if (mModel) {
            mScene->removeChild(mModel);
        }

        mModel = loadModel(sModelFiles[index]);
        mModelIndex = 0;
    }

    void gui() override
    {
        auto& frameUniforms = mView->frameUniforms();

        ImGui::SliderFloat("envIntensity", &frameUniforms.envIntensity, 0.0f, 10.0f);
        ImGui::SliderFloat("exposure", &frameUniforms.exposure, 0.0f, 10.0f);

        if (ImGui::BeginCombo("Model:", sModelFiles[mModelIndex])) {
            for (uint32_t i = 0; i < std::size(sModelFiles); i++) {
                if (ImGui::Selectable(sModelFiles[i], i == mModelIndex)) {
                    loadModelByIndex(i);
                }
            }

            ImGui::EndCombo();
        }
    }
};

DEF_DEMO(PbrDemo, "Pbr Demo");
}