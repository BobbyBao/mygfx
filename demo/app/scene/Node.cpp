#include "Node.h"

namespace mygfx {
	
	Node::Node() = default;

	Node& Node::position(const vec3& p) {
		mPosition = p;
		worldTransformDirty = true;
		return *this;
	}
		
	Node& Node::rotation(const quat& r) {
		mRotation = r;
		worldTransformDirty = true;
		return *this;
	}

	Node& Node::scale(const vec3& s) {
		mScale = s;
		worldTransformDirty = true;
		return *this;
	}

	const mat4& Node::getWorldTransform() const {
		if (worldTransformDirty) {
			updateTransform();
		}

		return worldTransform;
	}

	void Node::updateTransform() const {

		mat4 r = glm::scale_slow(glm::mat4_cast(mRotation), mScale);
		mat4 localTransform = glm::translate(r, mPosition);

		if (parent) {
			worldTransform = parent->getWorldTransform() * localTransform;
		} else {
			worldTransform = localTransform;
		}
	}
}