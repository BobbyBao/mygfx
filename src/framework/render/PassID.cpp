#include "PassID.h"
#include <shared_mutex>
#include <string>
#include <vector>

namespace mygfx {
namespace {
    HashMap<String, uint32_t> sPassNames { { "main", 0 } };
    std::shared_mutex sPassNameLock;
}

static uint32_t getPassID(const std::string_view& pass)
{
    if (pass.empty()) {
        return 0;
    }

    String passName(pass);
    {
        std::shared_lock<std::shared_mutex> lock(sPassNameLock);
        auto it = sPassNames.find(passName);
        if (it != sPassNames.end()) {
            return it->second;
        }
    }

    {
        uint32_t index = (uint32_t)sPassNames.size();
        std::unique_lock<std::shared_mutex> lock(sPassNameLock);
        sPassNames.try_emplace(passName, index);
        return index;
    }
}

PassID::PassID(const std::string_view& name)
{
    index = getPassID(name);
}

PassID PassID::Main("");
PassID PassID::Depth("depth");
PassID PassID::Shadow("shadow");

}