#include "Scene.h"

namespace mygfx {
	
	Scene::Scene() {
		mScene = this;
	}
		
	Node* Scene::instantiate(Node* model, Node* parent) {
		return nullptr;
	}
	
	Node* Scene::instantiate(Node* model, Node* parent, const vec3& pos, const Quaternion& rot, const vec3& scale) {
		return nullptr;
	}

	Node* Scene::instantiate(Node* model, const vec3& pos, const Quaternion& rot, const vec3& scale) {
		return nullptr;
	}
		
}