#pragma once

#include "Node.h"


namespace mygfx {

	class Camera : public Node {
	public:

		mat4 view;
		mat4 proj;
	};

}