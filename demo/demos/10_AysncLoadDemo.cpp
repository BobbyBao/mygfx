#include "SceneDemo.h"
#include "resource/Mesh.h"
#include "resource/Texture.h"
#if defined(_WIN32)

using namespace std::chrono_literals;

namespace mygfx::demo {

class AysncLoadDemo : public SceneDemo {
public:
    std::atomic<bool> mLoading = false;
    std::atomic<bool> mCancelLoad = false;

    AysncLoadDemo()
    {
    }

    void start() override
    {
        SceneDemo::start();

        mCameraController->lookAt(vec3 { 0.0f, 0.0f, 12.0f }, vec3 { 0.0f });

        load();
    }

    Result<void> load()
    {
        int GRID_SIZE = 5;
        float SPACE = 2.0f;
        float offset = (GRID_SIZE - 1) * SPACE / 2;
        mLoading = true;

        Ref<AysncLoadDemo> this_(this);

        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                auto model = co_await mApp->background_executor()->submit([this]() {
                    return MeshRenderable::createCube(1.0f);
                });

                co_await mApp->timer_queue()->make_delay_object(1000ms, mApp->background_executor());
                
                co_await resume_on(mApp->getMainExecutor());

                if (mCancelLoad) {
                    mLoading = false;
                    co_return;
                }

                if (mScene) {
                    mScene->instantiate(model, { i * SPACE - offset, j * SPACE - offset, 0.0f });
                }
            }
        }

        co_return;
    }

    void stop() override
    {
        SceneDemo::stop();

        mCancelLoad = true;
    }

};

DEF_DEMO(AysncLoadDemo, "Aysnc Load Demo");

}

#endif