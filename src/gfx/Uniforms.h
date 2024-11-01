#pragma once
#include <stdint.h>

namespace mygfx {

class Uniforms {
public:
    static constexpr int MAX_COUNT = 4;
    
    Uniforms() = default;

    Uniforms(uint32_t v0)
    {
        push_back(v0);
    }

    Uniforms(uint32_t v0, uint32_t v1)
    {
        push_back(v0);
        push_back(v1);
    }

    Uniforms(uint32_t v0, uint32_t v1, uint32_t v2)
    {
        push_back(v0);
        push_back(v1);
        push_back(v2);
    }

    Uniforms(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
    {
        push_back(v0);
        push_back(v1);
        push_back(v2);
        push_back(v3);
    }
    
    inline Uniforms(std::initializer_list<uint32_t> const& list)
    {
        for (auto& v : list) {
            push_back(v);
        }
    }

    void set(uint32_t v0)
    {
        mSize = 0;
        push_back(v0);
    }

    void set(uint32_t v0, uint32_t v1)
    {
        mSize = 0;
        push_back(v0);
        push_back(v1);
    }

    void set(uint32_t v0, uint32_t v1, uint32_t v2)
    {
        mSize = 0;
        push_back(v0);
        push_back(v1);
        push_back(v2);
    }

    void set(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
    {
        mSize = 0;
        push_back(v0);
        push_back(v1);
        push_back(v2);
        push_back(v3);
    }

    uint32_t size() const
    {
        return mSize;
    }

    const uint32_t& operator[](uint32_t index) const
    {
        assert(index < mSize);
        return mValues[index];
    }

    uint32_t& operator[](uint32_t index)
    {
        assert(index < mSize);
        return mValues[index];
    }

    const uint32_t* data() const { return &mValues[0]; }
    uint32_t* data() { return &mValues[0]; }

    inline void push_back(uint32_t v)
    {
        if (v != INVALID_UNIFORM_OFFSET) {
            mValues[mSize++] = v;
        }
    }

    inline void pop_back()
    {
        if (mSize > 0) {
            --mSize;
        }
    }

    inline void clear()
    {
        mSize = 0;
    }

private:
    uint32_t mValues[MAX_COUNT] = { 0 };
    uint32_t mSize = 0;
};


}