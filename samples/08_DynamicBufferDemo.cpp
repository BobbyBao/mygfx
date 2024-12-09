#include "VulkanExample.h"
#include "resource/Texture.h"
#include <array>

namespace mygfx::samples {

class DynamicBufferDemo : public Demo {
public:
    struct Vertex2D {
        float3 pos;
        uint32_t color;
        float2 uv;
        uint32_t texIndex;
    };

    struct Sprite2D {
        float2 position;
        float2 extend;
        float depth;
        uint32_t color;
        math::Rect uv;
        int32_t index;
        float2 destPos;

        uint16_t vertexStart;

        void getVertex(Vertex2D* v)
        {
            math::Rect r = { position - extend, position + extend };
            v[0].pos = { r.min, depth };
            v[0].color = color;
            v[0].uv = uv.min;
            v[0].texIndex = index;

            v[1].pos = { r.max.x, r.min.y, depth };
            v[1].color = color;
            v[1].uv = { uv.max.x, uv.min.y };
            v[1].texIndex = index;

            v[2].pos = { r.max, depth };
            v[2].color = color;
            v[2].uv = uv.max;
            v[2].texIndex = index;

            v[3].pos = { r.min.x, r.max.y, depth };
            v[3].color = color;
            v[3].uv = { uv.min.x, uv.max.y };
            v[3].texIndex = index;
        }
    };

    Vector<Vertex2D> mVertices;
    Vector<uint16_t> mIndices;
    Ref<Shader> mShader;
    Vector<Sprite2D> mSprites;
    Vector<Ref<Texture>> mTextures;

    void start() override
    {
        mShader = ShaderLibs::getUnlitShader();

        const int SPRITE_COUNT = 10000;

        mSprites.clear();
        mTextures = Texture::createRandomColorTextures(64);

        float width = (float)mApp->width;
        float height = (float)mApp->height;

        for (int i = 0; i < SPRITE_COUNT; i++) {

            float2 center = { linearRand<float>(0, width), linearRand<float>(0.0f, height) };
            float halfSize = linearRand<float>(5.0f, 10.0f);
            int index = mTextures[linearRand<int>(0, (int)mTextures.size() - 1)]->index();
            mSprites.push_back(
                Sprite2D { center, { halfSize, halfSize },
                    0.0f, 0xffffffff, math::Rect { { 0, 0 }, { 1, 1 } }, index, { linearRand<float>(0, width), linearRand<float>(0.0f, height) } });
        }

        for (auto& spr : mSprites) {
            createQuad(spr);
        }

    }

    void createQuad(Sprite2D& spr)
    {
        uint16_t start = (uint16_t)mVertices.size();
        mVertices.resize(start + 4);
        spr.getVertex(&mVertices[start]);
        spr.vertexStart = start;

        std::array<uint16_t, 6> indices;
        indices[0] = start;
        indices[1] = start + 1;
        indices[2] = start + 2;
        indices[3] = start + 3;
        indices[4] = start;
        indices[5] = start + 2;

        mIndices.insert(mIndices.end(), indices.begin(), indices.end());
    }

    void setPosition(Sprite2D& spr, const float2& pos)
    {
        spr.position = pos;

        Vertex2D* v = &mVertices[spr.vertexStart];
        math::Rect r = { pos - spr.extend, pos + spr.extend };
        v[0].pos = { r.min, spr.depth };
        v[1].pos = { r.max.x, r.min.y, spr.depth };
        v[2].pos = { r.max, spr.depth };
        v[3].pos = { r.min.x, r.max.y, spr.depth };
    }

    void update(float delta) override
    {
        auto w = (float)mApp->width;
        auto h = (float)mApp->height;

        for (auto& spr : mSprites) {
            auto dir = spr.destPos - spr.position;
            if (length(dir) < 5) {
                spr.destPos = { linearRand<float>(0, w), linearRand<float>(0.0f, h) };
            } else {
                dir = normalize(spr.destPos - spr.position);
            }

            setPosition(spr, spr.position + dir * (float)(50.0 * delta));
        }
    }

    void draw(GraphicsApi& cmd) override
    {
        if (mIndices.size() > 0) {
            float w = (float)mApp->width;
            float h = (float)mApp->height;
            Matrix4 proj = ortho(0.0f, w, h, 0.0f, -1.0f, 1.0f);
            uint32_t ubo = cmd.allocConstant(proj);

            cmd.bindPipelineState(mShader->pipelineState);
            cmd.bindUniforms({ ubo });
            cmd.drawUserPrimitives(std::span { mVertices }, std::span { mIndices });
        }
    }
};

DEF_DEMO(DynamicBufferDemo, "DynamicBuffer Demo");
}