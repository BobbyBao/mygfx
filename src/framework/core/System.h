#pragma once

#include "Object.h"

namespace mygfx {

class System : public Object {
public:
    System();

    virtual void init();
    virtual void shutdown();

protected:
};

template <typename T>
class TSystem : public System {
public:
    TSystem()
    {
        sInstance = static_cast<T*>(this);
    }

    ~TSystem()
    {
        sInstance = nullptr;
    }

    static T* getInstance()
    {
        return sInstance;
    }

protected:
    inline static T* sInstance = nullptr;
};

}