#include "Scene.h"

namespace mygfx {
	
	Scene::Scene() {
		mScene = this;
	}
		
	Node* Scene::instantiate(Node* model, Node* parent) {
		Node* newNode = model->clone();
		if (parent) {
			parent->addChild(newNode);
		} else {
			addChild(newNode);
		}
		return newNode;
	}
	
	Node* Scene::instantiate(Node* model, Node* parent, const vec3& pos, const Quaternion& rot, const vec3& scale) {
		Node* newNode = model->clone();
		newNode->setTRS(scale, rot, scale);
		if (parent) {
			parent->addChild(newNode);
		}
		else {
			addChild(newNode);
		}
		return newNode;
	}

	Node* Scene::instantiate(Node* model, const vec3& pos, const Quaternion& rot, const vec3& scale) {
		Node* newNode = model->clone();
		newNode->setTRS(scale, rot, scale);	
		addChild(newNode);		
		return newNode;
	}
		
}