#pragma once
#include "Core/Object.h"
#include "core/Maths.h"
#include "utils/RefCounted.h"
#include "GraphicsDefs.h"
#include "RenderQueue.h"

namespace mygfx {

class Scene;
class Camera;
class Texture;
class RenderGraph;

struct FrameUniforms {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    mat4 invViewMatrix;
    mat4 invProjectionMatrix;
    mat4 invViewProjectionMatrix;

    vec3 camera;
    float nearZ;
    vec3 cameraDir;
    float farZ;

    ivec2 screenSize;
    vec2 invScreenSize;

    float envIntensity = 1.0f;
    float exposure = 1.0f;
    int mipCount = 1;
    float pad2;

    int lambertianEnvTexture;
    int ggxEnvTexture;
    int ggxLUTTexture;
    float pad3;

    int charlieEnvTexture;
    int charlieLUTTexture;
    int sheenELUTTexture;
    float pad4;

    mat3x4 envRotation = identity<mat3x4>();
};

class View : public Object {
public:
    View(uint16_t width, uint16_t height, Format format, TextureUsage usage, SampleCount msaa);
    View(HwSwapchain* swapChain);

    void setScene(Scene* scene);
    void setCamera(Camera* camera);

    void setEnvIntensity(float envIntensity)
    {
        mFrameUniforms.envIntensity = envIntensity;
    }
    
    PROPERTY_GET_SET(RenderGraph*, RenderGraph)

    Texture* getRenderTexture() { return mRenderTexture; }

    auto& frameUniforms() { return mFrameUniforms; }

    void update(double delta);
    void render(GraphicsApi& cmd);

protected:
    Ref<HwSwapchain> mSwapchain;
    Ref<Texture> mRenderTexture;
    Ref<HwRenderTarget> mRenderTarget;
    Ref<Scene> mScene;
    Ref<Camera> mCamera;
    Ref<RenderGraph> mRenderGraph;
    FrameUniforms mFrameUniforms;    
    RenderQueue mRenderQueue;

};

}