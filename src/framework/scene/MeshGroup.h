#pragma once

#include "MeshRenderable.h"

namespace mygfx {

class Texture;
class Shader;

class MeshGroup : public MeshRenderable {
public:
    MeshGroup();

protected:
    Object* createObject() override;
    void cloneProcess(Object* destNode) override;
    void onAddToScene(Scene* scene) override;
    void onRemoveFromScene(Scene* scene) override;

};

}