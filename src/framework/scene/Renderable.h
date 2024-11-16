#pragma once
#include "Component.h"
#include "GraphicsConsts.h"
#include "core/Maths.h"

namespace mygfx {

class GraphicsApi;
struct RenderingContext;
class HwRenderPrimitive;
class Material;
class IndirectBuffer;

struct Primitive {
    HwRenderPrimitive* renderPrimitive = nullptr;
    Material* material = nullptr;
    uint32_t instanceCount = 1;
    uint32_t primitiveUniforms = INVALID_UNIFORM_OFFSET;
};

class IRenderer {
public:
    virtual void draw(GraphicsApi& cmd, RenderingContext& ctx) = 0;
};

class Renderable : public Component {
public:
    Renderable();

    PROPERTY_GET_SET(uint32_t, RenderableType)
    
    IRenderer* getRenderer() { return mRenderer; }

    Vector<Primitive> primitives;
    uint64_t transformBuffer = 0;
protected:
    void cloneProcess(Object* destObj) override;
    void onAddToScene(Scene* scene) override;
    void onRemoveFromScene(Scene* scene) override;
    IRenderer* mRenderer = nullptr;
    uint32_t mRenderableType = 0;
    mutable Aabb mBoundingBox;
    mutable bool mSkinning : 1 = false;
    mutable bool mMorphing : 1 = false;
};

class CustomRenderable : public Renderable, public IRenderer {
public:
    CustomRenderable();

};

}