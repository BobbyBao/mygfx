#pragma once

#include "utils/SharedPtr.h"
#include <vector>
#include <string>

#pragma warning(push)
#pragma warning(disable:4819)
#include <robin_hood/robin_hood.h>
#pragma warning(pop)

namespace mygfx {

	
	using namespace utils;

	using String = std::string;

	template<typename T>
	using Vector = std::vector<T>;

	using ByteArray = std::vector<uint8_t>;

	template<typename K, typename V>
	using HashMap = robin_hood::unordered_map<K, V>;

	template<typename T>
	using HashSet = robin_hood::unordered_set<T>;
}