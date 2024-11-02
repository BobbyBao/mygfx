#include "PassID.h"
#include <shared_mutex>
#include <string>
#include <vector>

namespace mygfx {
namespace {
    HashMap<String, uint32_t> sPassNames { { "main", 0 } };
    std::shared_mutex sPassNameLock;
}

static uint32_t getPassID(const char* pass)
{
    if (pass == nullptr || pass[0] == 0) {
        return 0;
    }

    {
        std::shared_lock<std::shared_mutex> lock(sPassNameLock);
        auto it = sPassNames.find(pass);
        if (it != sPassNames.end()) {
            return it->second;
        }
    }

    {
        uint32_t index = (uint32_t)sPassNames.size();
        std::unique_lock<std::shared_mutex> lock(sPassNameLock);
        sPassNames.try_emplace(pass, index);
        return index;
    }
}

PassID::PassID(const char* name)
{
    index = getPassID(name);
}

PassID PassID::Main("");
PassID PassID::Depth("depth");
PassID PassID::Shadow("shadow");

}