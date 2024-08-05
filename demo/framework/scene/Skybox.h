#pragma once

#include "Renderable.h"


namespace mygfx {
	
	class Texture;

	class Skybox : public Renderable {
	public:
		Skybox();

		Texture* getCubeMap() const { return mCubeMap; }
		void setCubeMap(Texture* tex);
	protected:
		void addToScene() override;
		void removeFromScene() override;

		Ref<Texture> mCubeMap;
	};

}