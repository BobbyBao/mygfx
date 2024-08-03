#pragma once

#include "utils/SharedPtr.h"
#include <vector>
#include <string>


namespace mygfx {

	
	using namespace utils;

	using String = std::string;

	template<typename T>
	using Vector = std::vector<T>;

	using ByteArray = std::vector<uint8_t>;
}