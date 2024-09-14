#include "Scene.h"

namespace mygfx {

Scene::Scene()
{
    mScene = this;
}

Ref<Node> Scene::instantiate(Node* model, Node* parent)
{
    Ref<Node> newNode = staticCast<Node>(model->clone());
    if (parent) {
        parent->addChild(newNode);
    } else {
        addChild(newNode);
    }
    return newNode;
}

Ref<Node> Scene::instantiate(Node* model, Node* parent, const vec3& pos, const Quaternion& rot, const vec3& scale)
{
    Ref<Node> newNode = staticCast<Node>(model->clone());
    newNode->setTRS(pos, rot, scale);
    if (parent) {
        parent->addChild(newNode);
    } else {
        addChild(newNode);
    }
    return newNode;
}

Ref<Node> Scene::instantiate(Node* model, const vec3& pos, const Quaternion& rot, const vec3& scale)
{
    Ref<Node> newNode = staticCast<Node>(model->clone());
    newNode->setTRS(pos, rot, scale);
    addChild(newNode);
    return newNode;
}

}