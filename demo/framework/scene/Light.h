#pragma once

#include "Node.h"


namespace mygfx {
	
	class Light : public Node {
	public:
		Light();
	protected:
		Node* createNode() override;
		void cloneProcess(Node* destNode) override;
	};

}