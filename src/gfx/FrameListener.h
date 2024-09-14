#pragma once
#include <list>

namespace mygfx {

class FrameChangeListener {
public:
    FrameChangeListener()
    {
        sCallables.push_back(this);
    }

    ~FrameChangeListener()
    {
        sCallables.remove(this);
    }

    virtual void onFrameChange() { }

    static void callFrameChange()
    {
        for (auto obj : sCallables) {
            obj->onFrameChange();
        }
    }

private:
    inline static std::list<FrameChangeListener*> sCallables;
};

}
