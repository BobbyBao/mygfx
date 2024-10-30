#pragma once
#include "Component.h"
#include "GraphicsConsts.h"

namespace mygfx {

class HwRenderPrimitive;
class Material;
class IndirectBuffer;

struct Primitive {
    HwRenderPrimitive* renderPrimitive = nullptr;
    IndirectBuffer* indirectBuffer = nullptr;
    Material* material = nullptr;
    uint32_t instanceCount = 1;
    uint32_t primitiveUniforms = INVALID_UNIFORM_OFFSET;
};

class Renderable abstract : public Component {
public:
    Renderable();

    PROPERTY_GET_SET(uint32_t, RenderQueue)

    Vector<Primitive> primitives;
protected:
    void onAddToScene(Scene* scene) override;
    void onRemoveFromScene(Scene* scene) override;

    uint32_t mRenderQueue = 0;
    mutable bool mSkinning : 1 = false;
    mutable bool mMorphing : 1 = false;
};

}