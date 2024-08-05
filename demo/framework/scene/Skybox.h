#pragma once

#include "Renderable.h"


namespace mygfx {
	
	class Skybox : public Renderable {
	public:
		Skybox();		
	protected:
		void addToScene() override;
		void removeFromScene() override;
	};

}