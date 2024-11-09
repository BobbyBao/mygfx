#pragma once
#include "core/Fwd.h"
#include "core/Maths.h"
#include "core/Object.h"

namespace mygfx {

class Component;
class Scene;

enum class TransformSpace {
    LOCAL = 0,
    PARENT,
    WORLD
};

class Node : public NamedObject {
public:
    Node();
    Node(const String& name, const vec3& pos = vec3 { 0 }, const quat& rot = identity<quat>(), const vec3& s = one<vec3>());

    void setActive(bool value);
    bool isActive() const { return mActive; }
    bool isActiveEffective() const { return mActive && mParentActive; }
    
    Node* getParent() const { return mParent; }
    Scene* getScene() const { return mScene; }

    template <typename T>
    T* getComponent() const
    {
        return (T*)getComponent(typeid(T));
    }

    template <typename T>
    T* getComponentBase()
    {
        for (auto& c : mComponents) {
            if (T* t = dynamic_cast<T*>(c.get())) {
                return t;
            }
        }

        return nullptr;
    }

    Component* getComponent(const std::type_info& typeInfo) const;
    
    template <typename T>
    T* findComponent() const
    {
        return (T*)findComponent(typeid(T));
    }

    Component* findComponent(const std::type_info& typeInfo) const;

    template <typename T, typename... Args>
    T* addComponent(Args... args)
    {
        T* c = new T(std::forward<Args>(args)...);
        addComponent(c);
        return c;
    }

    const auto& getComponents() const { return mComponents; }
    void addComponent(Component* c);

    template <typename T, typename... Args>
    T* createChild(Args... args)
    {
        T* so = new T(std::forward<Args>(args)...);
        addChild(so);
        return so;
    }

    template <typename T>
    T* createChild()
    {
        T* so = new T();
        addChild(so);
        return so;
    }

    void addChild(Node* child);
    void removeChild(Node* child);
    void remove();
    void setParent(Node* parent);

    PROPERTY_GET_1(vec3, Position)
    PROPERTY_GET_1(quat, Rotation)
    PROPERTY_GET_1(vec3, Scale)

    Node& setPosition(const vec3& p);
    Node& setRotation(const quat& r);
    Node& setScale(const vec3& s);

    vec3 getDirection() const;
    vec3 getUp() const;
    vec3 getSide() const;

    void setTRS(const vec3& p, const quat& r, const vec3& s, bool notifyChange = true);
    void setTransform(const mat4& m);
    void translate(const vec3& delta, TransformSpace space = TransformSpace::LOCAL);
    void lookAt(vec3 const& eye, vec3 const& center, vec3 const& up = { 0.0f, 1.0f, 0.0f }) noexcept;

    const vec3& getWorldPosition() const;
    quat getWorldRotation() const;
    const mat4& getWorldTransform() const;
    void updateTransform() const;

protected:
    virtual Object* createObject();
    virtual void cloneProcess(Object* destObj);
    void setScene(Scene* scene);
    virtual void addToScene();
    virtual void removeFromScene();
    virtual void onAddChild(Node* child);
    virtual void onRemoveChild(Node* child);
    void onSetActive(bool value);
    void onParentSetActive(bool value);
    void notifyActiveChanged();
    void hierarchyChanged();
    void transformChanged();
    virtual void onHierarchyChanged();
    virtual void onTransformChanged();

    Vector<Ref<Component>> mComponents;
    Vector<Ref<Node>> mChildren;
    Node* mParent = nullptr;
    Scene* mScene = nullptr;
    vec3 mPosition { 0.0f };
    quat mRotation = identity<quat>();
    vec3 mScale { 1.0f };
    mutable quat mWorldRotation;
    mutable mat4 mWorldTransform;
    bool mActive : 1 = true;
    bool mParentActive : 1 = true;
    bool mActiveEffective : 1 = true;
    mutable bool mWorldTransformDirty : 1 = true;
};

}