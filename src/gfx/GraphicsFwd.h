#pragma once

#include <memory>
#include <span>
#include <string>
#include <vector>

#include "utils/SharedPtr.h"

namespace utils {

using String = std::string;

template <typename T>
using Span = std::span<T>;

template <class T>
class SharedPtr;

template <class T>
using Ref = SharedPtr<T>;

}

namespace mygfx {

using namespace utils;

using String = std::string;

template <typename T>
using Span = std::span<T>;

template <typename T>
using Vector = std::vector<T>;

using ByteArray = std::vector<uint8_t>;

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