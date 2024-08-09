#pragma once

#include "GraphicsFwd.h"

#pragma warning(push)
#pragma warning(disable:4819)
#include <robin_hood/robin_hood.h>
#pragma warning(pop)

namespace mygfx {
    
	template<typename K, typename V>
	using HashMap = robin_hood::unordered_map<K, V>;

	template<typename T>
	using HashSet = robin_hood::unordered_set<T>;

}