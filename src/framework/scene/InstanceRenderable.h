#pragma once
#include "Renderable.h"
#include "core/Maths.h"

namespace mygfx {

class Mesh;

class InstanceBuffer : public RefCounted {
public:
    Vector<Ref<HwBuffer>> mInstanceBuffer;
    Ref<HwBuffer> mIndirectBuffer;
    uint32_t mMaxInstance = 0;
};

class InstanceRenderable : public Renderable {
public:
    InstanceRenderable();

    struct InstanceData {
        Vector3 position = zero<Vector3>();
        float scale = 1.0f;
        Quaternion rotation = identity<Quaternion>();
    };

    PROPERTY_GET(Mesh*, Mesh)

    void setMesh(Mesh* mesh);

protected:
    Object* createObject() override;
    void cloneProcess(Object* destNode) override;
    void updateRenderable();

    Ref<Mesh> mMesh;
    Ref<Material> mMaterial;
    Vector<Vector<InstanceData>> mInstanceData;
    Ref<InstanceBuffer> mInstanceBuffer;
};

}