#include "InstanceRenderable.h"
#include "Scene.h"
#include "resource/Material.h"
#include "resource/Mesh.h"
#include "resource/Shader.h"
#include "resource/Texture.h"

namespace mygfx {

InstanceRenderable::InstanceRenderable()
{
}

Object* InstanceRenderable::createObject()
{
    return new InstanceRenderable();
}

void InstanceRenderable::cloneProcess(Object* destNode)
{
    MeshRenderable::cloneProcess(destNode);

    InstanceRenderable* group = (InstanceRenderable*)destNode;
}

void InstanceRenderable::updateRenderable()
{
    MeshRenderable::updateRenderable();
}

}