#pragma once

#include "Node.h"


namespace mygfx {
	
	class Renderable;
	class Skybox;

	class Scene : public Node {
	public:
		Scene();
		
		Ref<Node> instantiate(Node* model, Node* parent = nullptr);
		Ref<Node> instantiate(Node* model, Node* parent, const vec3& pos, const Quaternion& rot = identity<Quaternion>(), const vec3& scale = vec3(1.0f));
		Ref<Node> instantiate(Node* model, const vec3& pos, const Quaternion& rot = identity<Quaternion>(), const vec3& scale = vec3(1.0f));
		
		
		HashSet<Renderable*> renderables;
		Skybox* skybox = nullptr;
	protected:
	};

}