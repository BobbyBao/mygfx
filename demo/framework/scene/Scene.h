#pragma once

#include "Node.h"


namespace mygfx {
	
	class Renderable;

	class Scene : public Node {
	public:
		Scene();
		
		Node* instantiate(Node* model, Node* parent = nullptr);
		Node* instantiate(Node* model, Node* parent, const vec3& pos, const Quaternion& rot = identity<Quaternion>(), const vec3& scale = vec3(1.0f));
		Node* instantiate(Node* model, const vec3& pos, const Quaternion& rot = identity<Quaternion>(), const vec3& scale = vec3(1.0f));
		
		
		HashSet<Renderable*> renderables;
	protected:
	};

}