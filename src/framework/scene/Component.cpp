#include "Component.h"
#include "Node.h"

namespace mygfx {

Component::Component() = default;

Scene* Component::getScene() const
{
    return mOwner ? mOwner->getScene() : nullptr;
}

void Component::cloneProcess(Object* destObj)
{
    Component* dest = static_cast<Component*>(destObj);

    dest->mEnable = mEnable;
    dest->mEnabledEffective = mEnabledEffective;
}

void Component::onActive()
{
}

void Component::onDeactive()
{
}

void Component::setOwner(Node* node)
{
    mOwner = node;

    onSetOwner(node);
}

void Component::onSetOwner(Node* node)
{
}

void Component::onActiveChanged()
{
    checkActivateState();
}

void Component::setEnabled(bool enable)
{
    if (enable != mEnable) {
        mEnable = enable;
        checkActivateState();
    }
}

void Component::checkActivateState()
{
    bool enabled = mEnable;
    if (mOwner) {
        enabled &= mOwner->isActiveEffective();
    }

    if (enabled != mEnabledEffective) {
        mEnabledEffective = enabled;
        if (getScene()) {
            if (enabled) {
                onActive();
            } else {
                onDeactive();
            }
        }
    }
}

void Component::onAddToScene(Scene* scene)
{
}

void Component::onRemoveFromScene(Scene* scene)
{
}

void Component::onParentChanged(Node* parent)
{
}

void Component::onTransformChanged()
{
}

}