#include "PassName.h"
#include <shared_mutex>
#include <string>
#include <vector>

namespace mygfx {
namespace {
    Vector<String> sPassNames { "main" };
    HashMap<std::string_view, uint32_t> sPassNameToIndex { { sPassNames[0], 0 } };
    std::shared_mutex sPassNameLock;
}

static uint32_t getPassID(const std::string_view& pass)
{
    if (pass.empty()) {
        return 0;
    }

    {
        std::shared_lock<std::shared_mutex> lock(sPassNameLock);
        auto it = sPassNameToIndex.find(pass);
        if (it != sPassNameToIndex.end()) {
            return it->second;
        }
    }

    {
        std::unique_lock<std::shared_mutex> lock(sPassNameLock);
        uint32_t index = (uint32_t)sPassNames.size();
        auto& name = sPassNames.emplace_back(pass);
        sPassNameToIndex.try_emplace(name, index);
        return index;
    }
}

PassName PassName::Main("");
PassName PassName::Depth("depth");
PassName PassName::Shadow("shadow");

PassName::PassName(const std::string_view& name)
{
    index = getPassID(name);
}

const String& PassName::getName() const
{
    return sPassNames[index];
}
}