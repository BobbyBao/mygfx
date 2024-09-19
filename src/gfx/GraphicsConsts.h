#pragma once
#include <stdint.h>

#define SINGLE_LOOP 0
#define SHOW_STATS 0
#define LARGE_DYNAMIC_INDEX 0
#define RESOURCE_STATE 1

namespace mygfx {

static constexpr uint32_t MAX_BACKBUFFER_COUNT = 3;
static constexpr bool INVERTED_DEPTH = true;
static constexpr bool LINEAR_COLOR_OUTPUT = true;
static constexpr uint32_t INVALID_UNIFORM_OFFSET = 0xffffffff;
}