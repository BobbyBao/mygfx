#pragma once
#include "core/Fwd.h"
#include <stdint.h>

namespace mygfx {

static const int MAX_PASS = 64;

struct PassName {
    PassName() = default;
    PassName(const std::string_view& name);

    inline operator uint32_t() { return index; }

    inline bool operator==(const PassName& rhs) const
    {
        return index == rhs.index;
    }

    const String& getName() const;

    static PassName Main;
    static PassName Depth;
    static PassName Shadow;

    uint32_t index = 0;
};

inline PassName operator""_pass(const char* _Str, size_t _Len) noexcept
{
    return PassName(std::string_view { _Str, _Len });
}

}

namespace std {
template <>
struct hash<mygfx::PassName> {
    size_t operator()(const mygfx::PassName& _Keyval) const noexcept
    {
        return _Keyval.index;
    }
};
}