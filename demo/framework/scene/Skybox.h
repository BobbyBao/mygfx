#pragma once

#include "Renderable.h"


namespace mygfx {
	
	class Texture;

	class Skybox : public Renderable {
	public:
		Skybox();

		Texture* getCubeMap() const { return mCubeMap; }
		void setCubeMap(Texture* tex);

		Texture* getIrrMap() const { return mIrrMap; }
		void setIrrMap(Texture* tex);

		Texture* getGGXLUT() const { return mGGXLUT; }
	protected:
		void addToScene() override;
		void removeFromScene() override;

		Ref<Texture> mCubeMap;
		Ref<Texture> mIrrMap;
		Ref<Texture> mGGXLUT;
	};

}