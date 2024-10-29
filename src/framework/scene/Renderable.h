#pragma once
#include "Component.h"
#include "GraphicsConsts.h"

namespace mygfx {

class GraphicsApi;
class HwRenderPrimitive;
class Material;
class IndirectBuffer;

struct Primitive {
    HwRenderPrimitive* renderPrimitive = nullptr;
    IndirectBuffer* indirectBuffer = nullptr;
    Material* material = nullptr;
    uint16_t instanceCount = 1;
    uint16_t firstInstance = 0;
    uint32_t primitiveUniforms = INVALID_UNIFORM_OFFSET;
};

class ICustomRenderer {
public:
    virtual void draw(GraphicsApi& cmd, uint32_t perView) = 0;
};

class Renderable abstract : public Component {
public:
    Renderable();

    PROPERTY_GET_SET(uint32_t, RenderQueue)
    
    ICustomRenderer* getRenderer() { return mCustomRenderer; }

    Vector<Primitive> primitives;
protected:
    void cloneProcess(Object* destObj) override;
    void onAddToScene(Scene* scene) override;
    void onRemoveFromScene(Scene* scene) override;
    ICustomRenderer* mCustomRenderer = nullptr;
    uint32_t mRenderQueue = 0;
    mutable bool mSkinning : 1 = false;
    mutable bool mMorphing : 1 = false;
};

}