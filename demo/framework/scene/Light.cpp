#include "Light.h"

namespace mygfx {
	
	Light::Light() {
	}

	Node* Light::createNode() {
		return new Light();
	}

	void Light::cloneProcess(Node* destNode) {
		Light* light = (Light*)destNode;

	}
}