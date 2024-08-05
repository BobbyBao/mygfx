#pragma once

#include <memory>
#include <vector>
#include <string>
#include <span>

#pragma warning(push)
#pragma warning(disable:4819)
#include <robin_hood/robin_hood.h>
#pragma warning(pop)

#include "utils/SharedPtr.h"

namespace utils {
	
	using String = std::string;
	
    template<typename T>
    using Span = std::span<T>;
	
	template <class T>
	class SharedPtr;
	
	template <class T>
	using Ref = SharedPtr<T>;
	
	template<typename K, typename V>
	using HashMap = robin_hood::unordered_map<K, V>;

	template<typename T>
	using HashSet = robin_hood::unordered_set<T>;
}

namespace mygfx {

	
	using namespace utils;

	using String = std::string;
	
    template<typename T>
    using Span = std::span<T>;
	
	template<typename T>
	using Vector = std::vector<T>;

	using ByteArray = std::vector<uint8_t>;

	template<typename K, typename V>
	using HashMap = robin_hood::unordered_map<K, V>;

	template<typename T>
	using HashSet = robin_hood::unordered_set<T>;

	class HwBuffer;
	class HwTexture;
	class HwTextureView;
	class HwRenderTarget;
	class HwSwapchain;
	class HwShaderModule;
	class HwProgram;
	class HwDescriptorSet;
	class HwRenderPrimitive;
}