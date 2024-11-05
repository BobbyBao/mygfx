#pragma once
#include "core/Fwd.h"
#include <stdint.h>

namespace mygfx {
    
static const int MAX_PASS = 64;

struct PassID
{
	PassID() = default;
	PassID(const std::string_view& name);

	inline operator uint32_t() { return index; }

	static PassID Main;
	static PassID Depth;
	static PassID Shadow;

	uint32_t index = 0;
};

}