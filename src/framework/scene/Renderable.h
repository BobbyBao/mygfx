#pragma once
#include "Component.h"

namespace mygfx {

class HwRenderPrimitive;
class Material;
class InstanceBuffer;

struct Primitive {
    HwRenderPrimitive* renderPrimitive = nullptr;
    InstanceBuffer* instanceBuffer = nullptr;
    Material* material = nullptr;
    uint32_t primitiveUniforms = 0;
};

class Renderable abstract : public Component {
public:
    Renderable();

    Vector<Primitive> primitives;

protected:
    void onAddToScene(Scene* scene) override;
    void onRemoveFromScene(Scene* scene) override;

    mutable bool mSkinning : 1 = false;
    mutable bool mMorphing : 1 = false;
};



}