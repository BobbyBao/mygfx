#include "Scene.h"

namespace mygfx {
	
	Scene::Scene() {
		mScene = this;
	}
		
	Ref<Node> Scene::instantiate(Node* model, Node* parent) {
		auto newNode = model->clone();
		if (parent) {
			parent->addChild(newNode);
		} else {
			addChild(newNode);
		}
		return newNode;
	}
	
	Ref<Node> Scene::instantiate(Node* model, Node* parent, const vec3& pos, const Quaternion& rot, const vec3& scale) {
		auto newNode = model->clone();
		newNode->setTRS(pos, rot, scale);
		if (parent) {
			parent->addChild(newNode);
		}
		else {
			addChild(newNode);
		}
		return newNode;
	}

	Ref<Node> Scene::instantiate(Node* model, const vec3& pos, const Quaternion& rot, const vec3& scale) {
		auto newNode = model->clone();
		newNode->setTRS(pos, rot, scale);
		addChild(newNode);		
		return newNode;
	}
		
}