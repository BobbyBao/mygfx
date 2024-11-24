#include "DemoApp.h"

namespace mygfx::samples {

class TriangleDemo : public Demo {
public:
    Ref<HwBuffer> mVB;
    Ref<Shader> mShader;

    struct VertexColored {
        float3 pos;
        uint32_t color;
    };

    void start() override
    {
        mShader = ShaderLibs::getColorShader();

        VertexColored pos[] = {
            { { 0.0f, -1.0f, 0.0f }, 0xff0000ff },
            { { -1.0f, 1.0f, 0.0f }, 0xff00ff00 },
            { { 1.0f, 1.0f, 0.0f }, 0xffff0000 }
        };

        mVB = gfxApi().createBuffer1(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, 3, pos);
    }

    void draw(GraphicsApi& cmd) override
    {
        auto mvp = glm::ortho(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f);

        uint32_t perView = gfxApi().allocConstant(mvp);

        cmd.bindPipelineState(mShader->pipelineState);
        cmd.bindUniforms({ perView });
        cmd.bindVertexBuffer(0, mVB, 0);
        cmd.draw(3, 1, 0, 0);
    }
};

DEF_DEMO(TriangleDemo, "Triangle Demo");
}