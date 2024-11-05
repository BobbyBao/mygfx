#pragma once
#include "core/Fwd.h"
#include "utils/SharedPtr.h"

namespace mygfx {

class Object : public utils::RefCounted {
public:
    Object();

    inline const std::type_info& getTypeInfo() const
    {
        return typeid(*this);
    }

    virtual Ref<Object> clone();

    template <typename T>
    T* cast()
    {
        return dynamic_cast<T*>(this);
    }

protected:
    virtual Object* createObject();
    virtual void cloneProcess(Object* destObj);
};

class NamedObject : public Object {
public:
    NamedObject() = default;

    inline const String& getName() const { return mName; }
    virtual void setName(const std::string_view& name);
protected:
    String mName;
};

#define PROPERTY_GET_SET(type, prop)           \
    type get##prop() const { return m##prop; } \
    void set##prop(type v) { m##prop = v; }

#define PROPERTY_GET(type, prop) \
    type get##prop() const { return m##prop; }

#define PROPERTY_GET_SET_BOOL(type, prop)     \
    type is##prop() const { return m##prop; } \
    void set##prop(type v) { m##prop = v; }

#define PROPERTY_GET_BOOL(type, prop) \
    type is##prop() const { return m##prop; }

#define PROPERTY_GET_SET_1(type, prop)                \
    const type& get##prop() const { return m##prop; } \
    void set##prop(const type& v) { m##prop = v; }

#define PROPERTY_GET_1(type, prop) \
    const type& get##prop() const { return m##prop; }

}