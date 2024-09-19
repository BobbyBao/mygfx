#include "SceneDemo.h"
#include "resource/Mesh.h"
#include "resource/Texture.h"
#if defined(_WIN32)
#include <concurrencpp/concurrencpp.h>

using namespace std::chrono_literals;

namespace mygfx::demo {

using namespace concurrencpp;

template <typename T>
using Result = concurrencpp::result<T>;

class AysncLoadDemo : public SceneDemo, public concurrencpp::runtime {
public:
    std::shared_ptr<concurrencpp::manual_executor> mMainExecutor;

    AysncLoadDemo()
    {
        mMainExecutor = make_manual_executor();
    }

    const std::shared_ptr<concurrencpp::manual_executor>& getMainExecutor() const { return mMainExecutor; }

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

        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                auto model = co_await background_executor()->submit([this]() {
                    return MeshRenderable::createCube(1.0f);
                });

                co_await timer_queue()->make_delay_object(1000ms, background_executor());

                co_await resume_on(getMainExecutor());

                mScene->instantiate(model, { i * SPACE - offset, j * SPACE - offset, 0.0f });
            }
        }

        co_return;
    }

    void update(double delta) override
    {
        mMainExecutor->loop_once();

        SceneDemo::update(delta);
    }
};

DEF_DEMO(AysncLoadDemo, "Aysnc Load Demo");

}

#endif