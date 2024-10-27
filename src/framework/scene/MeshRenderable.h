#pragma once
#include "Renderable.h"

namespace mygfx {

class Mesh;

class MeshRenderable : public Renderable {
public:
    void setMesh(Mesh* m);

    static Ref<Node> createCube(float size);

protected:
    Object* createObject() override;
    void cloneProcess(Object* destNode) override;
    virtual void updateRenderable();
    Ref<Mesh> mMesh;
};
}
