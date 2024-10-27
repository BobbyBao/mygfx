#pragma once
#include "core/Maths.h"
#include "MeshRenderable.h"

namespace mygfx {

class Texture;
class Shader;

class MeshGroup : public MeshRenderable {
public:
    MeshGroup();

    struct InstanceData {
        Vector3 position = zero<Vector3>();
        float scale = 1.0f;
        Quaternion rotation = identity<Quaternion>();
    };

protected:
    Object* createObject() override;
    void cloneProcess(Object* destNode) override;
    void updateRenderable() override;

    Vector<Vector<InstanceData>> mInstanceData;
};

}