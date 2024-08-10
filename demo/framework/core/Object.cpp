#include "Object.h"

namespace mygfx {

Object::Object() = default;

Ref<Object> Object::clone()
{
    Ref<Object> node(createObject());

    cloneProcess(node);

    return node;
}

Object* Object::createObject()
{
    return nullptr;
}

void Object::cloneProcess(Object* destNode)
{
}

void NamedObject::setName(const char* name)
{
    mName = name;
}

}