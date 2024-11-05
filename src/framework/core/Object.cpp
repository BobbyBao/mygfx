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

void Object::cloneProcess(Object* destObj)
{
}

void NamedObject::setName(const std::string_view& name)
{
    mName = name;
}

}