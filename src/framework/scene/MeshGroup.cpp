#include "MeshGroup.h"
#include "Scene.h"
#include "resource/Material.h"
#include "resource/Mesh.h"
#include "resource/Shader.h"
#include "resource/Texture.h"

namespace mygfx {

MeshGroup::MeshGroup()
{
}

Object* MeshGroup::createObject()
{
    return new MeshGroup();
}

void MeshGroup::cloneProcess(Object* destNode)
{
    MeshGroup* group = (MeshGroup*)destNode;
}

void MeshGroup::updateRenderable()
{

}

}