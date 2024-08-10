#include "Node.h"
#include "Component.h"
#include "Scene.h"

#include <glm/gtx/matrix_decompose.hpp>

namespace mygfx {

Node::Node() = default;

Node::Node(const String& n, const vec3& pos, const quat& rot, const vec3& s)
{
    mName = n;
    mPosition = pos;
    mRotation = rot;
    mScale = s;
}

void Node::addComponent(Component* component)
{
    mComponents.emplace_back(component);

    component->setOwner(this);

    if (mScene) {
        component->onAddToScene(mScene);
    }
}

void Node::setActive(bool value)
{
    mActive = value;

    if (mActiveEffective != isActiveEffective()) {
        mActiveEffective = isActiveEffective();
        notifyActiveChanged();
    }

    onSetActive(value);
}

void Node::onSetActive(bool value)
{
    for (auto& child : mChildren) {
        child->onParentSetActive(value);
    }
}

void Node::onParentSetActive(bool value)
{
    mParentActive = value;

    if (mActiveEffective != isActiveEffective()) {
        mActiveEffective = isActiveEffective();
        notifyActiveChanged();
    }

    for (auto& child : mChildren) {
        child->onParentSetActive(value && mActive);
    }
}

void Node::notifyActiveChanged()
{
    for (auto& component : mComponents) {
        component->onActiveChanged();
    }
}

Object* Node::createObject()
{
    return new Node();
}

void Node::cloneProcess(Object* destNode)
{
    Node* node = (Node*)destNode;
    node->mName = mName;
    node->mPosition = mPosition;
    node->mRotation = mRotation;
    node->mScale = mScale;
    node->mActive = mActive;

    for (auto& c : mComponents) {
        node->addComponent(dynamicCast<Component>(c->clone()));
    }

    for (auto& c : mChildren) {
        node->addChild(dynamicCast<Node>(c->clone()));
    }
}

void Node::addChild(Node* child)
{
    child->setParent(this);
}

void Node::removeChild(Node* child)
{
    if (child->mParent == this) {
        child->setParent(nullptr);
    }
}

void Node::remove()
{
    if (mParent) {
        mParent->removeChild(this);
    }
}

void Node::setParent(Node* parent)
{
    if (mParent == parent) {
        return;
    }

    if (mParent != nullptr) {
        mParent->onRemoveChild(this);
    }

    mParent = parent;

    if (mParent != nullptr) {
        mParent->onAddChild(this);
    }

    hierarchyChanged();
}

void Node::onAddChild(Node* child)
{
    mChildren.emplace_back(child);

    if (mScene != nullptr) {
        child->setScene(mScene);
    }
}

void Node::onRemoveChild(Node* child)
{
    auto it = std::find(mChildren.begin(), mChildren.end(), child);
    if (it != mChildren.end()) {
        mChildren.erase(it);
        child->setScene(nullptr);
    } else {
        assert(false);
    }
}

void Node::setScene(Scene* scene)
{
    if (mScene == scene) {
        return;
    }

    if (mScene != nullptr) {

        removeFromScene();

        for (auto& component : mComponents) {
            component->onRemoveFromScene(mScene);
        }
    }

    mScene = scene;

    if (mScene != nullptr) {

        addToScene();

        for (auto& component : mComponents) {
            component->onAddToScene(scene);
        }
    }

    for (auto& child : mChildren) {
        child->setScene(scene);
    }
}

void Node::addToScene()
{
}

void Node::removeFromScene()
{
}

void Node::hierarchyChanged()
{
    onHierarchyChanged();

    for (auto& component : mComponents) {
        component->onParentChanged(mParent);
    }

    transformChanged();
}

void Node::transformChanged()
{

    mWorldTransformDirty = true;

    onTransformChanged();

    for (auto& component : mComponents) {
        component->onTransformChanged();
    }

    for (auto& child : mChildren) {
        child->transformChanged();
    }
}

void Node::onHierarchyChanged()
{
}

void Node::onTransformChanged()
{
}

Node& Node::setPosition(const vec3& p)
{
    mPosition = p;
    transformChanged();
    return *this;
}

Node& Node::setRotation(const quat& r)
{
    mRotation = r;
    transformChanged();
    return *this;
}

Node& Node::setScale(const vec3& s)
{
    mScale = s;
    transformChanged();
    return *this;
}

vec3 Node::getDirection() const
{
    return getWorldRotation() * vec3(0.0f, 0.0f, -1.0f);
}

vec3 Node::getUp() const
{
    return getWorldRotation() * vec3(0.0f, 1.0f, 0.0f);
}

vec3 Node::getSide() const
{
    return getWorldRotation() * vec3(1.0f, 1.0f, 0.0f);
}

void Node::setTRS(const vec3& p, const quat& r, const vec3& s, bool notifyChange)
{
    mPosition = p;
    mRotation = r;
    mScale = s;

    if (notifyChange) {
        transformChanged();
    }
}

void Node::setTransform(const mat4& m)
{
    vec3 tr;
    quat r;
    vec3 s;
    vec3 skew;
    vec4 perspective;
    glm::decompose(m, s, r, tr, skew, perspective);

    setTRS(tr, r, s);
}

void Node::translate(const vec3& delta, TransformSpace space)
{
    switch (space) {
    case TransformSpace::LOCAL:
        // Note: local space translation disregards local scale for scale-independent movement speed
        mPosition += mRotation * delta;
        break;

    case TransformSpace::PARENT:
        mPosition += delta;
        break;

    case TransformSpace::WORLD:
        mPosition += (!mParent) ? delta : (inverse(mParent->getWorldTransform()) * vec4 { delta, 0.0f });
        break;
    }

    transformChanged();
}

void Node::lookAt(vec3 const& eye, vec3 const& center, vec3 const& up) noexcept
{
    mPosition = eye;
    vec3 dir = normalize(center - eye);
    mRotation = quatLookAtRH(dir, up);
    mScale = one<vec3>();
    transformChanged();
}

const vec3& Node::getWorldPosition() const
{
    if (mWorldTransformDirty) {
        updateTransform();
    }

    return *(const vec3*)&mWorldTransform[3][0];
}

quat Node::getWorldRotation() const
{
    if (mWorldTransformDirty) {
        updateTransform();
    }

    return mWorldRotation;
}

const mat4& Node::getWorldTransform() const
{
    if (mWorldTransformDirty) {
        updateTransform();
    }

    return mWorldTransform;
}

void Node::updateTransform() const
{

    mat4 localTransform = glm::scale(glm::mat4_cast(mRotation), mScale);
    localTransform[3] = { mPosition, 1.0f };

    if (mParent) {
        mWorldRotation = mParent->getWorldRotation() * mRotation;
        mWorldTransform = mParent->getWorldTransform() * localTransform;
    } else {
        mWorldRotation = mRotation;
        mWorldTransform = localTransform;
    }

    mWorldTransformDirty = false;
}
}