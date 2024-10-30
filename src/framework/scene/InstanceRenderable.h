#pragma once
#include "MeshRenderable.h"
#include "core/Maths.h"

namespace mygfx {

class Mesh;

class IndirectBuffer : public RefCounted {
public:
    PROPERTY_GET(HwBuffer*, IndirectBuffer)

    uint32_t drawCount = 0;

private:
    Ref<HwBuffer> mIndirectBuffer;
};

struct InstanceData {
    Vector3 position = zero<Vector3>();
    float scale = 1.0f;
    Quaternion rotation = identity<Quaternion>();
};

class InstanceRenderable : public MeshRenderable {
public:
    InstanceRenderable();

    void setInstanceData(Vector<Vector<InstanceData>>&& instanceData);

protected:
    Object* createObject() override;
    void cloneProcess(Object* destNode) override;
    void updateRenderable() override;
    void updateInstanceBuffers();

    Ref<Material> mMaterial;
    Vector<Vector<InstanceData>> mInstanceData;
    Vector<Ref<HwBuffer>> mInstanceBuffers;
    Ref<IndirectBuffer> mIndirectBuffer;
};

}